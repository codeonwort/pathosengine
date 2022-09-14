#version 460 core

#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D inPrevVisibility;
layout (binding = 1) uniform sampler2D inPrevHiZ;
layout (binding = 2) uniform sampler2D inCurrentHiZ;

// --------------------------------------------------------
// Output

layout (location = 0) out float outVisibility;

// --------------------------------------------------------
// Shader

// Calculates the percentage of empty space within the min/max depth ranges
// and modulates with the previous transparency.
void main() {
	vec2 screenUV = fs_in.screenUV;

	// Near is 0.0, far is 1.0 no matter if sceneDepth is Reverse-Z or not.
	vec4 fineZ = textureGather(inPrevHiZ, screenUV);
#if REVERSE_Z
	// Currently sceneDepthToLinearDepth() assumes Reverse-Z.
	fineZ = vec4(1.0) - fineZ;
#endif
	fineZ.x = sceneDepthToLinearDepth(screenUV, fineZ.x);
	fineZ.y = sceneDepthToLinearDepth(screenUV, fineZ.y);
	fineZ.z = sceneDepthToLinearDepth(screenUV, fineZ.z);
	fineZ.w = sceneDepthToLinearDepth(screenUV, fineZ.w);

	vec2 minmaxZ = texture(inCurrentHiZ, screenUV).xy;
#if REVERSE_Z
	minmaxZ = vec2(1.0) - minmaxZ;
#endif
	float minZ = sceneDepthToLinearDepth(screenUV, minmaxZ.x);
	float maxZ = sceneDepthToLinearDepth(screenUV, minmaxZ.y);

	float coarseVolume = 1.0 / (maxZ - minZ);

	vec4 visibility = textureGather(inPrevVisibility, screenUV);
	vec4 integration = (fineZ * abs(coarseVolume)) * visibility;

	float coarseIntegration = dot(vec4(0.25), integration);

	outVisibility = coarseIntegration;
}
