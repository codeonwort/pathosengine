#version 460 core

#include "core/common.glsl"
#include "core/brdf.glsl"
#include "core/image_based_lighting.glsl"
#include "core/diffuse_sh.glsl"
#include "deferred_common.glsl"

// #todo-light-probe: Finish indirect lighting shader
#define COSINE_WEIGHTED_INTERPOLATION    0
#define PROBE_VISIBILITY_AWARE           0

#define LIGHTINGMODE_NONE                0
#define LIGHTINGMODE_ALL                 1
#define LIGHTINGMODE_DIFFUSE_ONLY        2
#define LIGHTINGMODE_SKY_DIFFUSE_ONLY    3
#define LIGHTINGMODE_SPECULAR_ONLY       4

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
	uint  lightingMode; // cvar_indirectLighting

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

layout (std140, binding = 4) readonly buffer SSBO_SkyDiffuseSH {
	SHBuffer shBuffer;
} ssboSkyDiffuseSH;

layout (binding = 0) uniform usampler2D       gbufferA;
layout (binding = 1) uniform sampler2D        gbufferB;
layout (binding = 2) uniform usampler2D       gbufferC;
layout (binding = 3) uniform sampler2D        ssaoMap;
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

vec3 evaluateSkyDiffuse(vec3 dir) {
	return evaluateSH(ssboSkyDiffuseSH.shBuffer, dir).xyz;
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
void findIrradianceProbes(vec3 position, IrradianceVolume vol, out ProbeDesc probes[8]) {
	uvec3 maxGridCoord   = vol.gridSize - uvec3(1, 1, 1);
	vec3 fNumCells       = vec3(maxGridCoord);
	vec3 volSize         = vol.maxBounds - vol.minBounds;
	vec3 cellSize        = volSize / fNumCells;
	vec3 localUVW        = (position - vol.minBounds) / volSize;
	ivec3 firstGridCoord = clamp(ivec3(localUVW * fNumCells), ivec3(0, 0, 0), ivec3(maxGridCoord));
	// bvec && bvec is supported only on NVidia GPU? well shit :/
	//bvec3 axisIn         = lessThanEqual(vol.minBounds, position) && lessThan(position, vol.maxBounds);
	bvec3 axisIn         = bvec3(vol.minBounds.x <= position.x && position.x < vol.maxBounds.x, vol.minBounds.y <= position.y && position.y < vol.maxBounds.y, vol.minBounds.z <= position.z && position.z < vol.maxBounds.z);

	for (uint i = 0; i < 8; ++i) {
		uvec3 gridCoordOffset = uvec3(axisIn) * uvec3(i & 1, (i & 3) >> 1, i >> 2);
		uvec3 gridCoord       = uvec3(firstGridCoord) + gridCoordOffset;
		uint probeIx          = gridCoord.x + (gridCoord.y * vol.gridSize.x) + (gridCoord.z * vol.gridSize.x * vol.gridSize.y);
		probes[i].probeIx     = probeIx;
		probes[i].tileIx      = probeIx + vol.firstTileID;
		probes[i].center      = vol.minBounds + vec3(gridCoord) * cellSize;
	}
}

vec3 getIndirectDiffuse(GBufferData gbufferData, bool skyLightingOnly) {
	const vec3 surfacePositionWS = gbufferData.ws_coords;
	const vec3 surfaceNormalWS   = gbufferData.ws_normal;

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

	// If not in an irradiance volume, just return sky lighting.
	if (irradianceVolumeIndex == -1) {
		return ubo.skyLightBoost * evaluateSkyDiffuse(surfaceNormalWS);
	}

	const IrradianceVolume vol = ssbo0.irradianceVolumeInfo[irradianceVolumeIndex];
	ProbeDesc[8] probes;
	findIrradianceProbes(surfacePositionWS, vol, probes);

	vec4 samples[8];
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
		samples[i] = textureLod(irradianceProbeAtlas, atlasUV, 0);

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
		// dot() is for backface check
		visibility[i] = dot(surfaceNormalWS, probeToSurface) <= 0.0 && distanceToProbe - tolerance <= probeDepth;
		if (visibility[i] == false) {
			totalWeights -= weights[i];
			weights[i] = 0.0;
		}
#endif
	}

	vec3 irradiance = vec3(0.0);
#if COSINE_WEIGHTED_INTERPOLATION
	if (skyLightingOnly) {
		float skyOcclusion = 0.0;
		for (uint i = 0; i < 8; ++i) {
			skyOcclusion += samples[i].w * weights[i] / totalWeights;
		}
		skyOcclusion = min(1, skyOcclusion);
		irradiance = ubo.skyLightBoost * skyOcclusion * evaluateSkyDiffuse(surfaceNormalWS);
	} else {
		for (uint i = 0; i < 8; ++i) {
			irradiance += samples[i].xyz * weights[i] / totalWeights;
		}
		irradiance = max(vec3(0.0), ubo.diffuseBoost * irradiance);
	}
#else
	// Trilinear interpolation
	vec3 fNumCells = vec3(vol.gridSize - uvec3(1, 1, 1));
	vec3 volSize   = vol.maxBounds - vol.minBounds;
	vec3 cellSize  = volSize / fNumCells;
	vec3 ratio     = (surfacePositionWS - probes[0].center) / cellSize;

	if (skyLightingOnly) {
		float C00 = mix(samples[0].w, samples[1].w, ratio.x);
		float C01 = mix(samples[2].w, samples[3].w, ratio.x);
		float C10 = mix(samples[4].w, samples[5].w, ratio.x);
		float C11 = mix(samples[6].w, samples[7].w, ratio.x);
		float C0  = mix(C00, C01, ratio.y);
		float C1  = mix(C10, C11, ratio.y);
		float C   = mix(C0, C1, ratio.z); // skyOcclusion

		irradiance = ubo.skyLightBoost * C * evaluateSkyDiffuse(surfaceNormalWS);
	} else {
		vec3 C00 = mix(samples[0].xyz, samples[1].xyz, ratio.x);
		vec3 C01 = mix(samples[2].xyz, samples[3].xyz, ratio.x);
		vec3 C10 = mix(samples[4].xyz, samples[5].xyz, ratio.x);
		vec3 C11 = mix(samples[6].xyz, samples[7].xyz, ratio.x);
		vec3 C0  = mix(C00, C01, ratio.y);
		vec3 C1  = mix(C10, C11, ratio.y);
		vec3 C   = mix(C0, C1, ratio.z);

		irradiance = ubo.diffuseBoost * C;
	}
#endif

	return irradiance;
}

vec3 getIndirectSpecular(GBufferData gbufferData) {
	const vec3 surfacePositionWS   = gbufferData.ws_coords;
	const vec3 surfaceNormalWS     = gbufferData.ws_normal;
	const float roughness          = gbufferData.roughness;
	const vec3 viewDirectionWS     = normalize(uboPerFrame.cameraPositionWS - surfacePositionWS);
	const vec3 specularDirectionWS = reflect(-viewDirectionWS, surfaceNormalWS);

	// Find the closest reflection probe.
	int probeIndex = -1;
	float probeMinDist = 60000.0f;
	for (uint i = 0; i < ubo.numReflectionProbes; ++i) {
		ReflectionProbe probe = ssbo1.reflectionProbeInfo[i];
		float dist = length(surfacePositionWS - probe.positionWS);
		// #todo: Fetch probe depth and check if this reflection probe is visible from the surface point.
		if (dist <= probe.captureRadius && dist < probeMinDist) {
			dist = probeMinDist;
			probeIndex = int(i);
		}
	}

	// Sample the reflection probe.
	vec3 specularSample;
	if (probeIndex == -1) {
		specularSample = textureLod(skyReflectionProbe, specularDirectionWS, roughness * ubo.skyReflectionProbeMaxLOD).rgb;
		specularSample *= ubo.skyLightBoost;
	} else {
		vec4 R4 = vec4(specularDirectionWS, float(probeIndex));
		specularSample = textureLod(localRadianceCubeArray, R4, roughness * ubo.reflectionProbeMaxLOD).rgb;
		specularSample *= ubo.specularBoost;
		//if (probeIndex == 0) specularSample = vec3(10.0, 10.0, 0.0);
		//if (probeIndex == 1) specularSample = vec3(0.0, 10.0, 0.0);
		//if (probeIndex == 2) specularSample = vec3(0.0, 0.0, 10.0);
		//if (probeIndex == 3) specularSample = vec3(10.0, 0.0, 10.0);
	}

	return specularSample;
}

vec3 getImageBasedLighting(GBufferData gbufferData) {
	const vec3 surfacePositionWS   = gbufferData.ws_coords;
	const vec3 surfaceNormalWS     = gbufferData.ws_normal;
	const vec3 viewDirectionWS     = normalize(uboPerFrame.cameraPositionWS - surfacePositionWS);
	const float NdotV              = max(0.0, dot(surfaceNormalWS, viewDirectionWS));

	const vec3 albedo              = gbufferData.albedo;
	const float metallic           = gbufferData.metallic;
	const float roughness          = gbufferData.roughness;

	const vec3 F0                  = mix(vec3(0.04), min(albedo, vec3(1.0)), metallic);
	const vec3 F                   = fresnelSchlickRoughness(NdotV, F0, roughness);
	const vec3 kS                  = F;
	const vec3 kD                  = (1.0 - metallic) * (1.0 - kS);
	const vec2 envBRDF             = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;

	vec3 diffuseIndirect  = vec3(0.0);
	vec3 specularIndirect = vec3(0.0);
	uint lightingMode     = ubo.lightingMode;
	if (lightingMode == LIGHTINGMODE_ALL || lightingMode == LIGHTINGMODE_DIFFUSE_ONLY || lightingMode == LIGHTINGMODE_SKY_DIFFUSE_ONLY) {
		diffuseIndirect = getIndirectDiffuse(gbufferData, lightingMode == LIGHTINGMODE_SKY_DIFFUSE_ONLY);
	}
	if (lightingMode == LIGHTINGMODE_ALL || lightingMode == LIGHTINGMODE_SPECULAR_ONLY) {
		specularIndirect = getIndirectSpecular(gbufferData);
	}
	float ambientOcclusion = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 finalLighting = kD * albedo * diffuseIndirect;
	finalLighting += (kS * envBRDF.x + envBRDF.y) * specularIndirect;
	// NOTE: Applying occlusion after integrating over hemisphere is physically wrong.
	// #todo-light-probe: Specular occlusion for indirect specular
	finalLighting *= ambientOcclusion;

	return finalLighting;
}

vec3 getGlobalIllumination(GBufferData gbufferData) {
	const uint shadingModel = gbufferData.material_id;
	vec3 lighting = vec3(0.0);

	if (shadingModel == MATERIAL_SHADINGMODEL_DEFAULTLIT) {
		lighting.rgb = getImageBasedLighting(gbufferData);
	}

	return lighting;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbufferA, gbufferB, gbufferC, gbufferData);

	vec3 lighting = getGlobalIllumination(gbufferData);
	lighting = max(vec3(0.0), lighting);

	// Additive blending
	outSceneColor = vec4(lighting, 0.0);
}
