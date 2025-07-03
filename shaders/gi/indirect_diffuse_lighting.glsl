#version 460 core

#include "../core/common.glsl"
#include "../core/brdf.glsl"
#include "../core/image_based_lighting.glsl"
#include "../core/diffuse_sh.glsl"
#include "../deferred_common.glsl"

// #todo-light-probe: Finish indirect lighting shader
#define COSINE_WEIGHTED_INTERPOLATION    0
#define PROBE_VISIBILITY_AWARE           0
#define DDGI_LIKE_FILTERING              0

#define LIGHTINGMODE_NONE                0
#define LIGHTINGMODE_ALL                 1
#define LIGHTINGMODE_DIFFUSE_ONLY        2
#define LIGHTINGMODE_SKY_DIFFUSE_ONLY    3

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_IndirectDiffuseLighting {
	uint  lightingMode; // cvar_indirectLighting
	uint  numIrradianceVolumes;
	uint  irradianceTileCountX;
	uint  irradianceTileSize;

	float irradianceAtlasWidth;
	float irradianceAtlasHeight;
	float skyLightBoost;
	float diffuseBoost;
} ubo;

layout (std140, binding = 2) readonly buffer SSBO_IrradianceVolume {
	IrradianceVolume irradianceVolumeInfo[];
} ssbo0;

layout (std140, binding = 3) readonly buffer SSBO_SkyDiffuseSH {
	SHBuffer shBuffer;
} ssboSkyDiffuseSH;

layout (std140, binding = 4) readonly buffer SSBO_LightProbeSH {
	SHBuffer shBuffer[];
} ssboLightProbeSH;

layout (binding = 0) uniform usampler2D       gbufferA;
layout (binding = 1) uniform sampler2D        gbufferB;
layout (binding = 2) uniform usampler2D       gbufferC;
layout (binding = 3) uniform sampler2D        ssaoMap;
layout (binding = 4) uniform sampler2D        irradianceProbeAtlas;   // ONV-encoded irradiance maps from local irradiance probes
layout (binding = 5) uniform sampler2D        depthProbeAtlas;        // ONV-encoded linear depths from local irradiance probes

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

vec4 evaluateLightProbe(ProbeDesc probe, vec3 dir) {
#if 0
	vec4 uvBounds = getIrradianceTileBounds(probe.tileIx);
	vec2 encodedUV = ONVEncode(dir);
	vec2 atlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * encodedUV;

	return textureLod(irradianceProbeAtlas, atlasUV, 0);
#else
	SHBuffer sh = ssboLightProbeSH.shBuffer[probe.tileIx];
	return evaluateSH(sh, dir);
#endif
}

vec3 getProbeSpacing(IrradianceVolume vol) {
	uvec3 maxGridCoord   = vol.gridSize - uvec3(1, 1, 1);
	vec3 fNumCells       = vec3(maxGridCoord);
	vec3 volSize         = vol.maxBounds - vol.minBounds;
	vec3 cellSize        = volSize / fNumCells;
	return cellSize;
}

float getProbeDepth(ProbeDesc probe, vec3 direction) {
	vec4 uvBounds          = getIrradianceTileBounds(probe.tileIx);
	vec2 probeDepthLocalUV = ONVEncode(direction);
	vec2 probeDepthAtlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * probeDepthLocalUV;
	float probeDepth       = textureLod(depthProbeAtlas, probeDepthAtlasUV, 0).r;
	return probeDepth;
}

// #wip: DDGI
// https://github.com/NVIDIAGameWorks/RTXGI-DDGI/blob/main/rtxgi-sdk/shaders/ddgi/Irradiance.hlsl
vec3 DDGIGetVolumeIrradiance(vec3 worldPosition, vec3 surfaceBias, vec3 direction, IrradianceVolume volume) {
	vec3 irradiance = vec3(0.0);
	float accumWeights = 0.0;

	vec3 biasedWorldPosition = worldPosition + surfaceBias;

	ProbeDesc[8] probes;
	findIrradianceProbes(worldPosition, volume, probes);

	// Clamp the distance (in grid space) between the given point and the base probe's world position (on each axis) to [0, 1]
	vec3 gridSpaceDistance = (biasedWorldPosition - probes[0].center);
	vec3 alpha = clamp((gridSpaceDistance / getProbeSpacing(volume)), vec3(0.0), vec3(1.0));

	for (int i = 0; i < 8; i++) {
		ProbeDesc probe = probes[i];
		ivec3 adjacentProbeOffset = ivec3(i, i >> 1, i >> 2) & ivec3(1, 1, 1);

		vec3 worldPosToAdjProbe = normalize(probe.center - worldPosition);
		vec3 biasedPosToAdjProbe = normalize(probe.center - biasedWorldPosition);
		float biasedPosToAdjProbeDist = length(probe.center - biasedWorldPosition);

		// Compute trilinear weights based on the distance to each adjacent probe
		// to smoothly transition between probes. adjacentProbeOffset is binary, so we're
		// using a 1-alpha when adjacentProbeOffset = 0 and alpha when adjacentProbeOffset = 1.
		vec3 trilinear = max(vec3(0.001), mix(vec3(1 - alpha), vec3(alpha), adjacentProbeOffset));
		float trilinearWeight = (trilinear.x * trilinear.y * trilinear.z);
		float weight = 1.0;

		// A naive soft backface weight would ignore a probe when
		// it is behind the surface. That's good for walls, but for
		// small details inside of a room, the normals on the details
		// might rule out all of the probes that have mutual visibility 
		// to the point. We instead use a "wrap shading" test. The small
		// offset at the end reduces the "going to zero" impact.
		float wrapShading = (dot(worldPosToAdjProbe, direction) + 1.0) * 0.5;
		weight *= (wrapShading * wrapShading) + 0.2f;

#if PROBE_VISIBILITY_AWARE
		// Sample the probe's distance texture to get the mean distance to nearby surfaces
		vec2 filteredDistance = vec2(0, 0); // #wip

		// Find the variance of the mean distance
		float variance = abs((filteredDistance.x * filteredDistance.x) - filteredDistance.y);

		// Occlusion test
		float chebyshevWeight = 1.0;
		if (biasedPosToAdjProbeDist > filteredDistance.x) { // occluded
			// v must be greater than 0, which is guaranteed by the if condition above.
			float v = biasedPosToAdjProbeDist - filteredDistance.x;
			chebyshevWeight = variance / (variance + (v * v));

			// Increase the contrast in the weight
			chebyshevWeight = max((chebyshevWeight * chebyshevWeight * chebyshevWeight), 0.0);
		}

		// Avoid visibility weights ever going all the way to zero because
		// when *no* probe has visibility we need a fallback value
		weight *= max(0.05, chebyshevWeight);

		// Avoid a weight of zero
		weight = max(0.000001f, weight);

		//float probeDepth = getProbeDepth(probe, -worldPosToAdjProbe);
#endif

		// A small amount of light is visible due to logarithmic perception, so
		// crush tiny weights but keep the curve continuous
		const float crushThreshold = 0.2;
		if (weight < crushThreshold) {
			weight *= (weight * weight) * (1.0 / (crushThreshold * crushThreshold));
		}

		// Apply the trilinear weights
		weight *= trilinearWeight;

		// Sample the probe's irradiance
		vec4 probeIrradiance = evaluateLightProbe(probe, direction);

		// Decode the tone curve, but leave a gamma = 2 curve to approximate sRGB blending
		//vec3 exponent = volume.probeIrradianceEncodingGamma * 0.5; // #wip
		//irradiance = pow(irradiance, exponent);

		irradiance += (weight * probeIrradiance.xyz);
		accumWeights += weight;
	}

	if (accumWeights == 0.0) return vec3(0.0);

	irradiance *= 1.0 / accumWeights;
	irradiance *= irradiance;
	irradiance *= TWO_PI;

	return irradiance;
}

vec3 getIndirectDiffuse(GBufferData gbufferData, bool skyLightingOnly) {
	vec3 surfacePositionWS = gbufferData.ws_coords;
	vec3 surfaceNormalWS   = gbufferData.ws_normal;

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

	for (uint i = 0; i < 8; ++i) {
		ProbeDesc probe = probes[i];
		vec4 uvBounds = getIrradianceTileBounds(probe.tileIx);
		vec3 probeToSurface = normalize(surfacePositionWS - probe.center);
		float distanceToProbe = length(surfacePositionWS - probe.center);

		samples[i] = evaluateLightProbe(probe, surfaceNormalWS);
		weights[i] = max(0.0, dot(surfaceNormalWS, -probeToSurface));

#if PROBE_VISIBILITY_AWARE
		// Sample depth probe
		vec2 probeDepthLocalUV = ONVEncode(probeToSurface);
		vec2 probeDepthAtlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * probeDepthLocalUV;
		float probeDepth = textureLod(depthProbeAtlas, probeDepthAtlasUV, 0).r;

		// Test visibility
		const float tolerance = -0.05;
		// dot() is for backface check
		bool bVisible = dot(surfaceNormalWS, probeToSurface) <= 0.0 && distanceToProbe - tolerance <= probeDepth;
		if (!bVisible) weights[i] = 0;
#endif

		totalWeights += weights[i];
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
		vec3 L = vec3(0.0);
		for (uint i = 0; i < 8; ++i) {
			L += samples[i].xyz * weights[i] / totalWeights;
		}
		L = max(vec3(0.0), L);
		irradiance = ubo.diffuseBoost * L;
	}
#else
	// Trilinear interpolation
	vec3 fNumCells = vec3(vol.gridSize - uvec3(1, 1, 1));
	vec3 volSize   = vol.maxBounds - vol.minBounds;
	vec3 cellSize  = volSize / fNumCells;
	vec3 ratio     = (surfacePositionWS - probes[0].center) / cellSize;

	for (uint i = 0; i < 8; ++i) {
		samples[i] *= weights[i] / totalWeights;
	}

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

#if DDGI_LIKE_FILTERING
	vec3 L = DDGIGetVolumeIrradiance(surfacePositionWS, vec3(0.0), surfaceNormalWS, vol);
	irradiance = ubo.diffuseBoost * L / 100;
#endif

	return irradiance;
}

vec3 getGlobalIllumination_defaultLit(GBufferData gbufferData) {
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

	vec3 indirectDiffuse = vec3(0.0);
	uint lightingMode    = ubo.lightingMode;
	if (lightingMode == LIGHTINGMODE_ALL || lightingMode == LIGHTINGMODE_DIFFUSE_ONLY || lightingMode == LIGHTINGMODE_SKY_DIFFUSE_ONLY) {
		indirectDiffuse = getIndirectDiffuse(gbufferData, lightingMode == LIGHTINGMODE_SKY_DIFFUSE_ONLY);
	}
	float ambientOcclusion = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 finalLighting = kD * albedo * indirectDiffuse;
	// NOTE: Applying occlusion after integrating over hemisphere is physically wrong.
	finalLighting *= ambientOcclusion;

	return finalLighting;
}

vec3 getGlobalIllumination(GBufferData gbufferData) {
	const uint shadingModel = gbufferData.material_id;
	vec3 lighting = vec3(0.0);

	if (shadingModel == MATERIAL_SHADINGMODEL_DEFAULTLIT) {
		lighting.rgb = getGlobalIllumination_defaultLit(gbufferData);
	}

	return lighting;
}

// interleaved gradient noise
float noise_ig(ivec2 texel)
{
	const vec3 m = vec3(0.06711056, 0.0233486, 52.9829189);
	float theta = fract(m.z * fract(dot(vec2(texel), m.xy)));
	return theta;
}
vec2 VogelDiskOffset(int i, float phi)
{
	float r = sqrt(float(i) + 0.5) / sqrt(64.0);
	float theta = 2.4 * float(i) + phi;
	float x = r * cos(theta);
	float y = r * sin(theta);
	return vec2(x, y);
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbufferA, gbufferB, gbufferC, gbufferData);

	vec3 lighting = getGlobalIllumination(gbufferData);
	lighting = max(vec3(0.0), lighting);

	// Additive blending
	outSceneColor = vec4(lighting, 0.0);
}
