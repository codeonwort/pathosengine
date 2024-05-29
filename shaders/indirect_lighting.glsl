#version 460 core

#include "core/common.glsl"
#include "core/brdf.glsl"
#include "core/image_based_lighting.glsl"
#include "deferred_common.glsl"

// #todo-light-probe: Finish indirect lighting shader
#define COSINE_WEIGHTED_INTERPOLATION    0
#define PROBE_VISIBILITY_AWARE           0

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

layout (std140, binding = 2) readonly buffer SSBO_IrradianceVolume {
	IrradianceVolume irradianceVolumeInfo[];
} ssbo0;

layout (std140, binding = 3) readonly buffer SSBO_ReflectionProbe {
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
layout (binding = 8) uniform sampler2D        irradianceProbeAtlas;   // ONV-encoded irradiance maps from local irradiance probes
layout (binding = 9) uniform sampler2D        depthProbeAtlas;        // ONV-encoded linear depths from local irradiance probes

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

struct ProbeDesc {
	uint probeIx; // probe linear index in irradiance volume
	uint tileIx;  // corresponding tile index in probe atlas
	vec3 center;  // center position in world space
};

void getExtendedBounds(IrradianceVolume vol, out vec3 minBounds, out vec3 maxBounds) {
	vec3 fNumCells = vec3(vol.gridSize - uvec3(1, 1, 1));
	vec3 volSize = vol.maxBounds - vol.minBounds;
	vec3 cellSize = volSize / fNumCells;

	minBounds = vol.minBounds - cellSize;
	maxBounds = vol.maxBounds + cellSize;
}

bool isPositionInBounds(vec3 position, vec3 minBounds, vec3 maxBounds) {
	return all(lessThanEqual(minBounds, position)) && all(lessThan(position, maxBounds));
}

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

// If the position is in extended bounds, some probes are duplicated.
void findProbes(vec3 position, IrradianceVolume vol, out ProbeDesc probes[8]) {
	uvec3 maxGridCoord   = vol.gridSize - uvec3(1, 1, 1);
	vec3 fNumCells       = vec3(maxGridCoord);
	vec3 volSize         = vol.maxBounds - vol.minBounds;
	vec3 cellSize        = volSize / fNumCells;
	vec3 localUVW        = (position - vol.minBounds) / volSize;
	ivec3 firstGridCoord = clamp(ivec3(localUVW * fNumCells), ivec3(0, 0, 0), ivec3(maxGridCoord));
	bvec3 axisIn         = lessThanEqual(vol.minBounds, position) && lessThan(position, vol.maxBounds);

	for (uint i = 0; i < 8; ++i) {
		uvec3 gridCoordOffset = uvec3(axisIn) * uvec3(i & 1, (i & 3) >> 1, i >> 2);
		uvec3 gridCoord       = uvec3(firstGridCoord) + gridCoordOffset;
		uint probeIx          = gridCoord.x + (gridCoord.y * vol.gridSize.x) + (gridCoord.z * vol.gridSize.x * vol.gridSize.y);
		probes[i].probeIx     = probeIx;
		probes[i].tileIx      = probeIx + vol.firstTileID;
		probes[i].center      = vol.minBounds + vec3(gridCoord) * cellSize;
	}
}

vec3 getImageBasedLighting(GBufferData gbufferData) {
	const vec3 surfacePositionWS   = gbufferData.ws_coords;
	const vec3 surfaceNormalWS     = gbufferData.ws_normal;

	const vec3 viewDirectionWS     = normalize(uboPerFrame.cameraPositionWS - surfaceNormalWS);
	const vec3 specularDirectionWS = reflect(-viewDirectionWS, surfaceNormalWS);
	const float NdotV              = max(dot(surfaceNormalWS, viewDirectionWS), 0.0);

	const vec3 albedo              = gbufferData.albedo;
	const float metallic           = gbufferData.metallic;
	const float roughness          = gbufferData.roughness;
	const float localAO            = gbufferData.ao;

	const vec3 F0                  = mix(vec3(0.04), min(albedo, vec3(1.0)), metallic);
	const vec3 F                   = fresnelSchlickRoughness(NdotV, F0, roughness);
	const vec3 kS                  = F;
	const vec3 kD                  = (1.0 - metallic) * (1.0 - kS);

	//
	// Diffuse GI
	// 
	
	// Find an irradiance volume that contains the surface position.
	int irradianceVolumeIndex = -1;
	bool inExtendedBounds = false;
	for (uint i = 0; i < ubo.numIrradianceVolumes; ++i) {
		IrradianceVolume vol = ssbo0.irradianceVolumeInfo[i];
		if (isPositionInBounds(surfacePositionWS, vol.minBounds, vol.maxBounds)) {
			irradianceVolumeIndex = int(i);
			break;
		}
	}
	if (irradianceVolumeIndex == -1) {
		vec3 extendedMin, extendedMax;
		for (uint i = 0; i < ubo.numIrradianceVolumes; ++i) {
			getExtendedBounds(ssbo0.irradianceVolumeInfo[i], extendedMin, extendedMax);
			if (isPositionInBounds(surfacePositionWS, extendedMin, extendedMax)) {
				irradianceVolumeIndex = int(i);
				inExtendedBounds = true;
				break;
			}
		}
	}

	vec3 diffuseIndirect = vec3(0.0);
	if (irradianceVolumeIndex == -1) {
		// If not in an irradiance volume, just add sky lighting.
		diffuseIndirect = ubo.skyLightBoost * albedo * texture(skyIrradianceProbe, surfaceNormalWS).rgb;
	} else {
		const IrradianceVolume vol = ssbo0.irradianceVolumeInfo[irradianceVolumeIndex];
		ProbeDesc[8] probes;
		findProbes(surfacePositionWS, vol, probes);

		vec3 samples[8];
		float weights[8], totalWeights = 0.0;
#if PROBE_VISIBILITY_AWARE
		bool visibility[8];
#endif
		for (uint i = 0; i < 8; ++i) {
			ProbeDesc probe = probes[i];
			vec4 uvBounds = getIrradianceTileBounds(probe.tileIx);
			vec3 probeToSurface = normalize(surfacePositionWS - probe.center);

			// Sample irradiance probe
			vec2 encodedUV = ONVEncode(surfaceNormalWS);
			vec2 atlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * encodedUV;
			samples[i] = textureLod(irradianceProbeAtlas, atlasUV, 0).rgb;

			// Calculate probe weight
			weights[i] = max(0.0, dot(surfaceNormalWS, -probeToSurface));
			totalWeights += weights[i];

#if PROBE_VISIBILITY_AWARE
			// Sample depth probe
			vec2 probeDepthLocalUV = ONVEncode(probeToSurface);
			vec2 probeDepthAtlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * probeDepthLocalUV;
			float probeDepth = textureLod(depthProbeAtlas, probeDepthAtlasUV, 0).r;

			// Test visibility
			float distanceToProbe = length(surfacePositionWS - probe.center);
			const float tolerance = 0.005;
			visibility[i] = dot(surfaceNormalWS, probeToSurface) <= 0.0 && distanceToProbe - tolerance <= probeDepth; // dot() is for backface check
			if (visibility[i] == false) {
				totalWeights -= weights[i];
				weights[i] = 0.0;
			}
#endif
		}

#if COSINE_WEIGHTED_INTERPOLATION
		for (uint i = 0; i < 8; ++i) {
			diffuseIndirect += samples[i] * weights[i] / totalWeights;
		}
		diffuseIndirect = max(vec3(0.0), ubo.diffuseBoost * diffuseIndirect);
#else
		// Trilinear interpolation
		vec3 fNumCells = vec3(vol.gridSize - uvec3(1, 1, 1));
		vec3 volSize   = vol.maxBounds - vol.minBounds;
		vec3 cellSize  = volSize / fNumCells;
		vec3 ratio     = (surfacePositionWS - probes[0].center) / cellSize;

		vec3 C00 = mix(samples[0], samples[1], ratio.x);
		vec3 C01 = mix(samples[2], samples[3], ratio.x);
		vec3 C10 = mix(samples[4], samples[5], ratio.x);
		vec3 C11 = mix(samples[6], samples[7], ratio.x);
		vec3 C0 = mix(C00, C01, ratio.y);
		vec3 C1 = mix(C10, C11, ratio.y);
		diffuseIndirect = ubo.diffuseBoost * mix(C0, C1, ratio.z);
#endif
	}

	//
	// Specular GI
	// 
	
	// Select closest reflection probe.
	int localSpecularIndex = -1;
	float reflectionProbeMinDist = 60000.0f;
	for (uint i = 0; i < ubo.numReflectionProbes; ++i) {
		ReflectionProbe probe = ssbo1.reflectionProbeInfo[i];
		float dist = length(surfacePositionWS - probe.positionWS);
		// #todo: Fetch probe depth and check if this reflection probe is visible from the surface point.
		if (dist <= probe.captureRadius && dist < reflectionProbeMinDist) {
			dist = reflectionProbeMinDist;
			localSpecularIndex = int(i);
		}
	}
	vec3 specularSample;
	if (localSpecularIndex == -1) {
		specularSample = textureLod(skyReflectionProbe, specularDirectionWS, roughness * ubo.skyReflectionProbeMaxLOD).rgb;
		specularSample *= ubo.skyLightBoost;
	} else {
		vec4 R4 = vec4(specularDirectionWS, float(localSpecularIndex));
		specularSample = textureLod(localRadianceCubeArray, R4, roughness * ubo.reflectionProbeMaxLOD).rgb;
		specularSample *= ubo.specularBoost;
		//if (localSpecularIndex == 0) specularSample = vec3(10.0, 10.0, 0.0);
		//if (localSpecularIndex == 1) specularSample = vec3(0.0, 10.0, 0.0);
		//if (localSpecularIndex == 2) specularSample = vec3(0.0, 0.0, 10.0);
		//if (localSpecularIndex == 3) specularSample = vec3(10.0, 0.0, 10.0);
	}
	vec2 envBRDF          = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;
	vec3 specularIndirect = specularSample * (kS * envBRDF.x + envBRDF.y);

	//
	// Ambient occlusion
	// 
	
	// NOTE: Applying occlusion after integrating over hemisphere is physically wrong but that's a limit of IBL approaches.
	float ssaoSample = texture2D(ssaoMap, fs_in.screenUV).r;

	//
	// Final integration
	//

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
