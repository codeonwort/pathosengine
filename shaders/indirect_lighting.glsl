#version 460 core

#include "core/common.glsl"
#include "core/brdf.glsl"
#include "deferred_common.glsl"

// #todo-light-probe: SH encoding for indirect diffuse

struct IrradianceVolume {
	vec3 minBounds;
	uint firstTileID;
	vec3 maxBounds;
	uint numProbes;
	uvec3 gridSize;
	float captureRadius;
};

struct ReflectionProbe {
	vec3 positionWS;
	float captureRadius;
};

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_IndirectLighting {
	// Fake intensity controls
	float skyLightBoost;
	float diffuseBoost;
	float specularBoost;
	float _pad0;

	float skyReflectionProbeMaxLOD;
	float reflectionProbeMaxLOD; // Max LOD of local reflection probes
	uint numReflectionProbes;
	uint numIrradianceVolumes;

	float irradianceAtlasWidth;
	float irradianceAtlasHeight;
	uint irradianceTileCountX;
	uint irradianceTileSize;
} ubo;

layout (std140, binding = 2) readonly buffer SSBO_0_IndirectLighting {
	IrradianceVolume irradianceVolumeInfo[];
} ssbo0;

layout (std140, binding = 3) readonly buffer SSBO_1_IndirectLighting {
	ReflectionProbe reflectionProbeInfo[];
} ssbo1;

layout (binding = 0) uniform usampler2D       gbufferA;
layout (binding = 1) uniform sampler2D        gbufferB;
layout (binding = 2) uniform usampler2D       gbufferC;
layout (binding = 3) uniform sampler2D        ssaoMap;
layout (binding = 4) uniform samplerCube      skyIrradianceProbe;     // Sky diffuse IBL
layout (binding = 5) uniform samplerCube      skyReflectionProbe;     // Sky specular IBL
layout (binding = 6) uniform sampler2D        brdfIntegrationMap;     // Precomputed table for specular IBL
layout (binding = 7) uniform samplerCubeArray localRadianceCubeArray; // Prefiltered local specular IBLs
layout (binding = 8) uniform sampler2D        irradianceAtlas;        // ONV-encoded irradiance maps from local irradiance probes
layout (binding = 9) uniform sampler2D        depthProbeAtlas;        // ONV-encoded linear depths from local irradiance probes

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

// See scene.cpp
uvec2 getIrradianceTileTexel(uint tileID) {
	uint x = 1 + (tileID % ubo.irradianceTileCountX) * (2 + ubo.irradianceTileSize);
	uint y = 1 + (tileID / ubo.irradianceTileCountX) * (2 + ubo.irradianceTileSize);
	return uvec2(x, y);
}
vec4 getIrradianceTileBounds(uint tileID) {
	uvec2 texel = getIrradianceTileTexel(tileID);
	float dx = 0.5 / ubo.irradianceAtlasWidth;
	float dy = 0.5 / ubo.irradianceAtlasHeight;
	vec4 bounds;
	bounds.x = dx + texel.x / ubo.irradianceAtlasWidth;
	bounds.y = dy + texel.y / ubo.irradianceAtlasHeight;
	bounds.z = -dx + (texel.x + ubo.irradianceTileSize) / ubo.irradianceAtlasWidth;
	bounds.w = -dy + (texel.y + ubo.irradianceTileSize) / ubo.irradianceAtlasHeight;
	return bounds;
}

float remap(float x, float oldMin, float oldMax, float newMin, float newMax) {
	return newMin + (newMax - newMin) * (x - oldMin) / (oldMax - oldMin);
}

vec3 mixProbeSamples(vec3 L0, vec3 L1, bool b0, bool b1, float k) {
	if (b0 && b1) return mix(L0, L1, k);
	if (b0 && !b1) return L0;
	if (!b0 && b1) return L1;
	return vec3(1.0, 0.0, 0.0);
}

vec3 getImageBasedLighting(GBufferData gbufferData) {
	vec3 N = gbufferData.normal;
	vec3 V = normalize(uboPerFrame.eyePosition - gbufferData.vs_coords);
	float NdotV = max(dot(N, V), 0.0);

	vec3 N_world = gbufferData.ws_normal;
	vec3 V_world = normalize(uboPerFrame.ws_eyePosition - gbufferData.ws_coords);
	vec3 R_world = reflect(-V_world, N_world);

	vec3 albedo = gbufferData.albedo;
	float metallic = gbufferData.metallic;
	float roughness = gbufferData.roughness;
	float localAO = gbufferData.ao;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(albedo, vec3(1.0)), metallic);
	
	vec3 Lo = vec3(0.0);

	vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);

	vec3 kS = F;
	vec3 kD = (1.0 - metallic) * (1.0 - kS);

	// Diffuse GI
	int irradianceVolumeIndex = -1;
	for (uint i = 0; i < ubo.numIrradianceVolumes; ++i) {
		IrradianceVolume vol = ssbo0.irradianceVolumeInfo[i];
		bool bInside = all(lessThanEqual(vol.minBounds, gbufferData.ws_coords))
			&& all(lessThan(gbufferData.ws_coords, vol.maxBounds));

		if (bInside) {
			irradianceVolumeIndex = int(i);
			break;
		}
	}
	vec3 diffuseIndirect = vec3(0.0);
	if (irradianceVolumeIndex == -1) {
		diffuseIndirect = texture(skyIrradianceProbe, N_world).rgb * albedo;
		diffuseIndirect *= ubo.skyLightBoost;
	} else {
		// #todo: Move some calculations to UBO or SSBO
		IrradianceVolume vol = ssbo0.irradianceVolumeInfo[irradianceVolumeIndex];
		vec3 fNumCells = vec3(vol.gridSize - uvec3(1, 1, 1));
		vec3 volSize = vol.maxBounds - vol.minBounds;
		vec3 cellSize = volSize / fNumCells;
		vec3 relPos = (gbufferData.ws_coords - vol.minBounds) / volSize; // [0, 1)
		uvec3 minGridCoord = uvec3(relPos * fNumCells);
		
		vec3 diffuseSamples[8];
		bool diffuseSamplesValid[8];
		vec3 probeMinPos = vol.minBounds + vec3(minGridCoord) * cellSize;

// 0: Just leak
// 1: Zero value from non-visible probes, but just interpolate.
// 2: Completely ignore non-visible probes.
#define LIGHT_LEAK_SOLUTION 0

		for (uint i = 0; i < 8; ++i) {
			uvec3 gridCoordOffset = uvec3(i & 1, (i & 3) >> 1, i >> 2);
			uvec3 gridCoord = minGridCoord + gridCoordOffset;
			uint probeIx = gridCoord.x
				+ gridCoord.y * vol.gridSize.x
				+ gridCoord.z * (vol.gridSize.x * vol.gridSize.y);
			
			uint tileIx = probeIx + vol.firstTileID;
			vec2 encodedUV = ONVEncode(N_world);
			vec4 uvBounds = getIrradianceTileBounds(tileIx);
			vec2 atlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * encodedUV;

			diffuseSamples[i] = textureLod(irradianceAtlas, atlasUV, 0).rgb;
			
			vec3 probePos = vol.minBounds + vec3(gridCoord) * cellSize;
			vec3 N_probe = normalize(gbufferData.ws_coords - probePos);
			vec2 probeDepthLocalUV = ONVEncode(N_probe);
			vec2 probeDepthAtlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * probeDepthLocalUV;
			float probeDepth = textureLod(depthProbeAtlas, probeDepthAtlasUV, 0).r;

			diffuseSamplesValid[i] = dot(N_world, N_probe) <= 0.0 && length(gbufferData.ws_coords - probePos) <= probeDepth;
#if LIGHT_LEAK_SOLUTION == 1
			if (diffuseSamplesValid[i] == false) diffuseSamples[i] = vec3(0.0, 0.0, 0.0);
#endif
		}

		// Trilinear interpolation
		vec3 ratio = (gbufferData.ws_coords - probeMinPos) / cellSize;

#if LIGHT_LEAK_SOLUTION == 2
		// Dafuq is this
		vec3 C00 = mixProbeSamples(diffuseSamples[0], diffuseSamples[1], diffuseSamplesValid[0], diffuseSamplesValid[1], ratio.x);
		vec3 C01 = mixProbeSamples(diffuseSamples[2], diffuseSamples[3], diffuseSamplesValid[2], diffuseSamplesValid[3], ratio.x);
		vec3 C10 = mixProbeSamples(diffuseSamples[4], diffuseSamples[5], diffuseSamplesValid[4], diffuseSamplesValid[5], ratio.x);
		vec3 C11 = mixProbeSamples(diffuseSamples[6], diffuseSamples[7], diffuseSamplesValid[6], diffuseSamplesValid[7], ratio.x);
		vec3 C0 = mixProbeSamples(C00, C01, diffuseSamplesValid[0] || diffuseSamplesValid[1], diffuseSamplesValid[2] || diffuseSamplesValid[3], ratio.y);
		vec3 C1 = mixProbeSamples(C10, C11, diffuseSamplesValid[4] || diffuseSamplesValid[5], diffuseSamplesValid[6] || diffuseSamplesValid[7], ratio.y);
		vec3 C = mixProbeSamples(
			C0, C1,
			diffuseSamplesValid[0] || diffuseSamplesValid[1] || diffuseSamplesValid[2] || diffuseSamplesValid[3],
			diffuseSamplesValid[4] || diffuseSamplesValid[5] || diffuseSamplesValid[6] || diffuseSamplesValid[7],
			ratio.z);
#else
		vec3 C00 = mix(diffuseSamples[0], diffuseSamples[1], ratio.x);
		vec3 C01 = mix(diffuseSamples[2], diffuseSamples[3], ratio.x);
		vec3 C10 = mix(diffuseSamples[4], diffuseSamples[5], ratio.x);
		vec3 C11 = mix(diffuseSamples[6], diffuseSamples[7], ratio.x);
		vec3 C0 = mix(C00, C01, ratio.y);
		vec3 C1 = mix(C10, C11, ratio.y);
		vec3 C = mix(C0, C1, ratio.z);
#endif

		diffuseIndirect = C;
		diffuseIndirect *= ubo.diffuseBoost;
	}

	// Specular GI
	// Select closest reflection probe.
	int localSpecularIndex = -1;
	float reflectionProbeMinDist = 60000.0f;
	for (uint i = 0; i < ubo.numReflectionProbes; ++i) {
		ReflectionProbe probe = ssbo1.reflectionProbeInfo[i];
		float dist = length(gbufferData.ws_coords - probe.positionWS);
		// #todo: Fetch probe depth and check if this reflection probe is visible from the surface point.
		if (dist <= probe.captureRadius && dist < reflectionProbeMinDist) {
			dist = reflectionProbeMinDist;
			localSpecularIndex = int(i);
		}
	}
	vec3 specularSample;
	if (localSpecularIndex == -1) {
		specularSample = textureLod(skyReflectionProbe, R_world, roughness * ubo.skyReflectionProbeMaxLOD).rgb;
		specularSample *= ubo.skyLightBoost;
	} else {
		vec4 R4 = vec4(R_world, float(localSpecularIndex));
		specularSample = textureLod(localRadianceCubeArray, R4, roughness * ubo.reflectionProbeMaxLOD).rgb;
		specularSample *= ubo.specularBoost;
		//if (localSpecularIndex == 0) specularSample = vec3(10.0, 10.0, 0.0);
		//if (localSpecularIndex == 1) specularSample = vec3(0.0, 10.0, 0.0);
		//if (localSpecularIndex == 2) specularSample = vec3(0.0, 0.0, 10.0);
		//if (localSpecularIndex == 3) specularSample = vec3(10.0, 0.0, 10.0);
	}
	vec2 envBRDF          = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;
	vec3 specularIndirect = specularSample * (kS * envBRDF.x + envBRDF.y);

	// Ambient occlusion
	// NOTE: Applying occlusion AFTER integrating over hemisphere
	//       is physically wrong but that's a limit of IBL approaches.
	float ssaoSample = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 finalIrradiance = kD * albedo * diffuseIndirect;
	finalIrradiance += specularIndirect;
	finalIrradiance *= ssaoSample;

	return finalIrradiance;
}

vec3 getGlobalIllumination(GBufferData gbufferData) {
	uint shadingModel = gbufferData.material_id;
	vec3 irradiance = vec3(0.0);

	if (shadingModel == MATERIAL_SHADINGMODEL_DEFAULTLIT) {
		irradiance.rgb = getImageBasedLighting(gbufferData);
	}

	return irradiance;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbufferA, gbufferB, gbufferC, gbufferData);

	vec3 irradiance = getGlobalIllumination(gbufferData);
	irradiance.rgb = max(vec3(0.0), irradiance.rgb);

	outSceneColor = vec4(irradiance, 0.0);
}
