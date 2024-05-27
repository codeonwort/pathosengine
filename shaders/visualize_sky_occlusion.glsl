#version 460 core

#include "core/common.glsl"
#include "core/image_based_lighting.glsl"
#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} interpolants;

layout (std140, binding = 1) uniform UBO_VisualizeSkyOcclusion {
	uint  numIrradianceVolumes;
	float irradianceAtlasWidth;
	float irradianceAtlasHeight;
	uint  irradianceTileCountX;
	uint  irradianceTileSize;
} ubo;

layout (std140, binding = 2) buffer SSBO {
	IrradianceVolume irradianceVolumeInfo[];
} ssbo;

layout (binding = 0) uniform usampler2D gbufferA;
layout (binding = 1) uniform sampler2D  gbufferB;
layout (binding = 2) uniform usampler2D gbufferC;
layout (binding = 3) uniform sampler2D  irradianceProbeAtlas;
layout (binding = 4) uniform sampler2D  depthProbeAtlas;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outColor;

// --------------------------------------------------------
// Shader

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

float sampleSkyOcclusion(GBufferData gbufferData) {
	// Find an irradiance volume that contains the position.
	int irradianceVolumeIndex = -1;
	for (uint i = 0; i < ubo.numIrradianceVolumes; ++i) {
		IrradianceVolume vol = ssbo.irradianceVolumeInfo[i];
		bool bInside = all(lessThanEqual(vol.minBounds, gbufferData.ws_coords))
			&& all(lessThan(gbufferData.ws_coords, vol.maxBounds));
		if (bInside) {
			irradianceVolumeIndex = int(i);
			break;
		}
	}
	if (irradianceVolumeIndex == -1) {
		return 1.0;
	}

	IrradianceVolume vol = ssbo.irradianceVolumeInfo[irradianceVolumeIndex];
	vec3 fNumCells       = vec3(vol.gridSize - uvec3(1, 1, 1));
	vec3 volSize         = vol.maxBounds - vol.minBounds;
	vec3 cellSize        = volSize / fNumCells;
	vec3 relPos          = (gbufferData.ws_coords - vol.minBounds) / volSize; // [0, 1)
	uvec3 minGridCoord   = uvec3(relPos * fNumCells);
	vec3 probeMinPos     = vol.minBounds + vec3(minGridCoord) * cellSize;

	float samples[8];
	for (uint i = 0; i < 8; ++i) {
		uvec3 gridCoordOffset = uvec3(i & 1, (i & 3) >> 1, i >> 2);
		uvec3 gridCoord = minGridCoord + gridCoordOffset;
		uint probeIx = gridCoord.x
			+ gridCoord.y * vol.gridSize.x
			+ gridCoord.z * (vol.gridSize.x * vol.gridSize.y);
		uint tileIx = probeIx + vol.firstTileID;
		vec2 encodedUV = ONVEncode(gbufferData.ws_normal);
		vec4 uvBounds = getIrradianceTileBounds(tileIx);
		vec2 atlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * encodedUV;

		samples[i] = textureLod(irradianceProbeAtlas, atlasUV, 0).w;
	}

	// Trilinear interpolation
	vec3 ratio = (gbufferData.ws_coords - probeMinPos) / cellSize;
	float C00 = mix(samples[0], samples[1], ratio.x);
	float C01 = mix(samples[2], samples[3], ratio.x);
	float C10 = mix(samples[4], samples[5], ratio.x);
	float C11 = mix(samples[6], samples[7], ratio.x);
	float C0 = mix(C00, C01, ratio.y);
	float C1 = mix(C10, C11, ratio.y);
	float C = mix(C0, C1, ratio.z);

	// #wip-skyocclusion: Prevent the resource from being compiled out
	C += max(textureLod(depthProbeAtlas, vec2(0.0), 0).r, 1e-16);

	return C;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbufferA, gbufferB, gbufferC, gbufferData);

	float occlusion = sampleSkyOcclusion(gbufferData);

	outColor = vec4(occlusion.xxx, 1);
}
