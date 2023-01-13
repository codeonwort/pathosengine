// Visualize light probes

#version 460 core

// --------------------------------------------------------
// Common

#include "common.glsl"
#include "deferred_common.glsl"

#if VERTEX_SHADER
	#define INTERPOLANTS_QUALIFIER out
#elif FRAGMENT_SHADER
	#define INTERPOLANTS_QUALIFIER in
#endif

INTERPOLANTS_QUALIFIER Interpolants {
	flat uint instanceID; // gl_InstanceID
	flat uint irradianceVolumeIndex; // Irradiance volume only
	flat uint irradianceProbeIndex; // Irradiance volume only
	vec3 positionWS;
	vec3 normalWS;
} interpolants;

struct IrradianceVolume {
	vec3 minBounds;
	uint firstTileID;
	vec3 maxBounds;
	uint numProbes;
	uvec3 gridSize;
	uint _pad0;
};
struct RadianceProbe {
	vec3 positionWS;
	float captureRadius;
};

layout (std140, binding = 1) uniform UBO_VisualizeLightProbe {
	uint numIrradianceVolumes;
	uint totalIrradianceProbes;
	uint numRadianceProbes;
	float irradianceProbeRadius;

	float radianceProbeRadius;
	float irradianceAtlasWidth;
	float irradianceAtlasHeight;
	uint irradianceTileCountX;

	uint irradianceTileSize;
} ubo;

layout (std140, binding = 2) buffer SSBO_0 {
	IrradianceVolume volumeArray[];
} ssbo0;

layout (std140, binding = 3) buffer SSBO_1 {
	RadianceProbe probeArray[];
} ssbo1;

uvec3 getGridCoord(uint probeIndex, uvec3 gridSize) {
	uint sizeXY = gridSize.x * gridSize.y;
	uint gridZ = probeIndex / sizeXY;
	uint k = probeIndex - gridZ * sizeXY;
	uint gridY = k / gridSize.x;
	uint gridX = k % gridSize.x;
	return uvec3(gridX, gridY, gridZ);
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

// --------------------------------------------------------
// Vertex Shader

#if VERTEX_SHADER

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec3 inNormal;

void main() {
	vec3 center = vec3(0.0);
	float scale = 1.0;
	
	uint volumeIndex = 0;
	uint probeIndex = 0;
	if (gl_InstanceID < ubo.totalIrradianceProbes) {
		uint probesSoFar = 0;
		for (uint ix = 0; ix < ubo.numIrradianceVolumes; ++ix) {
			if (gl_InstanceID < probesSoFar + ssbo0.volumeArray[ix].numProbes) {
				volumeIndex = ix;
				probeIndex = gl_InstanceID - probesSoFar;
				break;
			}
			probesSoFar += ssbo0.volumeArray[ix].numProbes;
		}

		IrradianceVolume volume = ssbo0.volumeArray[volumeIndex];
		uvec3 gridCoord = getGridCoord(probeIndex, volume.gridSize);
		vec3 cellSize = (volume.maxBounds - volume.minBounds) / vec3(volume.gridSize - uvec3(1, 1, 1));
		vec3 probePos = volume.minBounds + cellSize * vec3(gridCoord);

		center = probePos;
		scale = ubo.irradianceProbeRadius;
	} else {
		center = ssbo1.probeArray[gl_InstanceID - ubo.totalIrradianceProbes].positionWS;
		scale = ubo.radianceProbeRadius;
		volumeIndex = probeIndex = 0xffffffff;
	}

	vec4 positionWS = vec4(inPosition, 1.0);
	positionWS.xyz = positionWS.xyz * scale + center;

	interpolants.instanceID = gl_InstanceID;
	interpolants.irradianceVolumeIndex = volumeIndex;
	interpolants.irradianceProbeIndex = probeIndex;
	interpolants.positionWS = positionWS.xyz;
	interpolants.normalWS = inNormal;

	gl_Position = uboPerFrame.viewProjTransform * positionWS;
}

#endif // VERTEX_SHADER

// --------------------------------------------------------
// Fragment Shader

#if FRAGMENT_SHADER

layout (binding = 0) uniform sampler2D irradianceAtlas;
layout (binding = 1) uniform samplerCubeArray radianceCubeArray;

layout (location = 0) out vec4 outColor;

void main() {
	vec3 N = normalize(interpolants.normalWS);
	uint instanceID = interpolants.instanceID;

	vec4 debugColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (instanceID < ubo.totalIrradianceProbes) {
		IrradianceVolume volume = ssbo0.volumeArray[interpolants.irradianceVolumeIndex];
		uint tileID = interpolants.irradianceProbeIndex + volume.firstTileID;
		vec4 uvBounds = getIrradianceTileBounds(tileID);

		vec2 encodedUV = ONVEncode(N);
		vec2 atlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * encodedUV;
		debugColor.rgb = textureLod(irradianceAtlas, atlasUV, 0).rgb;
	} else {
		uint probeIndex = instanceID - ubo.totalIrradianceProbes;
		vec4 R = vec4(N, float(probeIndex));
		debugColor.rgb = textureLod(radianceCubeArray, R, 0).rgb;
	}

	outColor = debugColor;
}

#endif // FRAGMENT_SHADER
