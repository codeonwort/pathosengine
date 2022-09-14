#version 460 core

#include "deferred_common.glsl"

// SSAO two pass depth-aware gaussian blur (kernel size is fixed to 3)

#if HORIZONTAL != 0 && HORIZONTAL != 1
	#error "Define HORIZONTAL as 0 or 1"
#endif

#define DEPTH_AWARE 1

layout (binding = 0) uniform sampler2D inSSAO;
layout (binding = 1) uniform sampler2D inSceneDepth;

in VS_OUT {
	vec2 screenUV;
} fs_in;

#define KERNEL_SIZE 3 // Actual kernel size is (KERNEL_SIZE * 2 - 1)
const float gaussWeight[KERNEL_SIZE] = float[] (0.38774, 0.24477, 0.06136);

out vec4 outBlurredSSAO;

void main() {
	vec2 uv = fs_in.screenUV;
	vec2 invTexSize = 1.0 / vec2(textureSize(inSSAO, 0));

	vec3 sourceColor = textureLod(inSSAO, uv, 0).rgb;
	float sourceDepth = textureLod(inSceneDepth, uv, 0).r;
	vec3 result = sourceColor * gaussWeight[0];

	for(int i = 1; i < KERNEL_SIZE; ++i) {
#if HORIZONTAL
		vec2 delta = vec2(i * invTexSize.x, 0.0);
#else
		vec2 delta = vec2(0.0, i * invTexSize.y);
#endif
		vec3 sampleR = textureLod(inSSAO, uv + delta, 0).rgb;
		vec3 sampleL = textureLod(inSSAO, uv - delta, 0).rgb;

#if DEPTH_AWARE
		// #todo: Convert to linear depths?
		float depthR = textureLod(inSceneDepth, uv + delta, 0).r;
		float depthL = textureLod(inSceneDepth, uv - delta, 0).r;
		float depthDiffR = 0.1 * abs(sourceDepth - depthR);
		float depthDiffL = 0.1 * abs(sourceDepth - depthL);
		depthDiffR *= depthDiffR;
		depthDiffL *= depthDiffL;

		result += sampleR * gaussWeight[i] / (1.0 + depthDiffR);
		result += sampleL * gaussWeight[i] / (1.0 + depthDiffL);
#else
		result += sampleR * gaussWeight[i];
		result += sampleL * gaussWeight[i];
#endif
	}

	outBlurredSSAO = vec4(result, 0.0);
}
