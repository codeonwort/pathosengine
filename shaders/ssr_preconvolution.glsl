#version 460 core

#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D inPrevMip;

// --------------------------------------------------------
// Output

layout (location = 0) out vec3 outSceneColor;

// --------------------------------------------------------
// Shader

#if FIRST_MIP

// Just generates a half res version of original scene color.
void main() {
	vec2 screenUV = fs_in.screenUV;
	vec3 sceneColor = texture(inPrevMip, screenUV).rgb;
	outSceneColor = sceneColor;
}

#else // FIRST_MIP

#define KERNEL_RADIUS 3
const float weights[KERNEL_RADIUS + 1] = float[](0.474, 0.233, 0.028, 0.001);

void main() {
	vec2 screenUV = fs_in.screenUV;
	vec2 invInputSize = 1.0 / vec2(textureSize(inPrevMip, 0));

	vec3 result = weights[0] * texture(inPrevMip, screenUV).rgb;

#if HORIZONTAL
	for (int k = 1; k <= KERNEL_RADIUS; ++k) {
		vec2 deltaUV = vec2(k * invInputSize.x, 0.0);
		result += weights[k] * texture(inPrevMip, screenUV - deltaUV).rgb;
		result += weights[k] * texture(inPrevMip, screenUV + deltaUV).rgb;
	}
#else
	for (int k = 1; k <= KERNEL_RADIUS; ++k) {
		vec2 deltaUV = vec2(0.0, k * invInputSize.y);
		result += weights[k] * texture(inPrevMip, screenUV - deltaUV).rgb;
		result += weights[k] * texture(inPrevMip, screenUV + deltaUV).rgb;
	}
#endif

	outSceneColor = result;
}

#endif // FIRST_MIP
