#version 460 core

#include "../core/common.glsl"
#include "../core/diffuse_sh.glsl"
#include "../core/image_based_lighting.glsl"
#include "../deferred_common.glsl"

// #todo-skyocclusion: Hard cut between grid cells
#define COSINE_WEIGHTED_INTERPOLATION    1

// #todo-skyocclusion: Can't make it right :(
#define PROBE_VISIBILITY_AWARE           1

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} interpolants;

layout (std140, binding = 1) uniform UBO_VisualizeIndirectDiffuse {
	uint  numIrradianceVolumes;
	float irradianceAtlasWidth;
	float irradianceAtlasHeight;
	uint  irradianceTileCountX;
	uint  irradianceTileSize;
} ubo;

layout (std140, binding = 2) buffer SSBO {
	IrradianceVolume irradianceVolumeInfo[];
} ssbo;

layout (std140, binding = 3) buffer SSBO_SHBuffer {
	SHBuffer shBuffer[];
};

layout (binding = 0) uniform usampler2D gbufferA;
layout (binding = 1) uniform sampler2D  gbufferB;
layout (binding = 2) uniform usampler2D gbufferC;
layout (binding = 3) uniform sampler2D  sceneDepth;
layout (binding = 4) uniform sampler2D  irradianceProbeAtlas;
layout (binding = 5) uniform sampler2D  depthProbeAtlas;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outColor;

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

float sampleProbe(ProbeDesc probe, vec3 surfaceNormal) {
#if 0
	vec4 uvBounds  = getIrradianceTileBounds(probe.tileIx);
	vec2 encodedUV = ONVEncode(surfaceNormal);
	vec2 atlasUV   = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * encodedUV;
	return textureLod(irradianceProbeAtlas, atlasUV, 0).w;
#else
	return evaluateSH(shBuffer[probe.tileIx], surfaceNormal).w;
#endif
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

float sampleSkyOcclusion(GBufferData gbufferData) {
	const vec3 surfacePosition = gbufferData.ws_coords;
	const vec3 surfaceNormal = gbufferData.ws_normal;

	// Find an irradiance volume that contains the surface position.
	int irradianceVolumeIndex = -1;
	bool inExtendedBounds = false;
	for (uint i = 0; i < ubo.numIrradianceVolumes; ++i) {
		IrradianceVolume vol = ssbo.irradianceVolumeInfo[i];
		if (isPositionInBounds(surfacePosition, vol.minBounds, vol.maxBounds)) {
			irradianceVolumeIndex = int(i);
			break;
		}
	}
	if (irradianceVolumeIndex == -1) {
		vec3 extendedMin, extendedMax;
		for (uint i = 0; i < ubo.numIrradianceVolumes; ++i) {
			getExtendedBounds(ssbo.irradianceVolumeInfo[i], extendedMin, extendedMax);
			if (isPositionInBounds(surfacePosition, extendedMin, extendedMax)) {
				irradianceVolumeIndex = int(i);
				inExtendedBounds = true;
				break;
			}
		}
	}
	
	// Exit if even not in an extended bounds.
	if (irradianceVolumeIndex == -1) {
		return 1.0;
	}

	const IrradianceVolume vol = ssbo.irradianceVolumeInfo[irradianceVolumeIndex];
	ProbeDesc[8] probes;
	findProbes(surfacePosition, vol, probes);

	float samples[8], weights[8], totalWeights = 0.0;
#if PROBE_VISIBILITY_AWARE
	bool visibility[8];
#endif
	for (uint i = 0; i < 8; ++i) {
		ProbeDesc probe = probes[i];
		vec4 uvBounds = getIrradianceTileBounds(probe.tileIx);
		vec3 probeToSurface = normalize(surfacePosition - probe.center);

		// Sample irradiance probe
		samples[i] = sampleProbe(probe, surfaceNormal);

		// Calculate probe weight
		weights[i] = max(0.0, dot(surfaceNormal, -probeToSurface));
		totalWeights += weights[i];

#if PROBE_VISIBILITY_AWARE
		// Sample depth probe
		vec2 probeDepthLocalUV = ONVEncode(probeToSurface);
		vec2 probeDepthAtlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * probeDepthLocalUV;
		float probeDepth       = textureLod(depthProbeAtlas, probeDepthAtlasUV, 0).r;

		// Test visibility
		float distanceToProbe = length(surfacePosition - probe.center);
		const float tolerance = 0.005;
		visibility[i] = dot(surfaceNormal, probeToSurface) <= 0.0 && distanceToProbe - tolerance <= probeDepth; // dot() is for backface check
#endif
	}
	
	float skyOcclusion = 0.0;

#if COSINE_WEIGHTED_INTERPOLATION
	for (uint i = 0; i < 8; ++i) {
		skyOcclusion += samples[i] * weights[i] / totalWeights;
	}
	skyOcclusion = min(1, skyOcclusion); // if totalWeights == 0.0, make it fully visible.
#else
	// Trilinear interpolation
	vec3 fNumCells = vec3(vol.gridSize - uvec3(1, 1, 1));
	vec3 volSize   = vol.maxBounds - vol.minBounds;
	vec3 cellSize  = volSize / fNumCells;
	vec3 ratio     = (surfacePosition - probes[0].center) / cellSize;

	float C00 = mix(samples[0], samples[1], ratio.x);
	float C01 = mix(samples[2], samples[3], ratio.x);
	float C10 = mix(samples[4], samples[5], ratio.x);
	float C11 = mix(samples[6], samples[7], ratio.x);
	float C0 = mix(C00, C01, ratio.y);
	float C1 = mix(C10, C11, ratio.y);
	skyOcclusion = mix(C0, C1, ratio.z);
#endif

	return skyOcclusion;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbufferA, gbufferB, gbufferC, gbufferData);

	float deviceZ = texture(sceneDepth, interpolants.screenUV).r;
	float occlusion = (deviceZ == 0.0) ? 1.0 : sampleSkyOcclusion(gbufferData);

	outColor = vec4(occlusion.xxx, 1);
}
