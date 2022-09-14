#version 460 core

#if HORIZONTAL != 0 && HORIZONTAL != 1
	#error "Define HORIZONTAL as 0 or 1"
#endif

layout (binding = 0) uniform sampler2D src;
#if ADDITIVE
// For bloom
layout (binding = 1) uniform sampler2D additive;
layout (location = 0) uniform float additiveWeight;
#endif

in VS_OUT {
	vec2 screenUV;
} fs_in;

// Supports 3, 5, 6, 7
#ifndef KERNEL_SIZE
	#define KERNEL_SIZE 5 // Actual kernel size is (KERNEL_SIZE * 2 - 1)
#endif

// #todo-gaussian: Receive weights as uniform (KERNEL_SIZE with shader permutation)
#if KERNEL_SIZE == 3
	const float weight[KERNEL_SIZE] = float[] (0.38774, 0.24477, 0.06136);
#elif KERNEL_SIZE == 5
	// I forgot where I did get this value, but (sigma = 1.7515, kernel size = 9) closely matches this setup.
	const float weight[KERNEL_SIZE] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
#elif KERNEL_SIZE == 6
	const float weight[KERNEL_SIZE] = float[] (0.225096, 0.192081, 0.119348, 0.053988, 0.017776, 0.004259);
#elif KERNEL_SIZE == 7
	//const float weight[KERNEL_SIZE] = float[] (0.224762, 0.191796, 0.119171, 0.053908, 0.01775, 0.004253, 0.000741);
	//const float weight[KERNEL_SIZE] = float[] (0.098779, 0.09683, 0.091209, 0.082557, 0.071805, 0.060013, 0.048196);
	const float weight[KERNEL_SIZE] = float[] (0.082339, 0.081929, 0.08071, 0.078719, 0.076014, 0.072672, 0.068786);
#else
	#error "KERNEL_SIZE is wrong"
#endif

out vec4 out_color;

void main() {
	vec2 uv = fs_in.screenUV;
	vec2 invTexSize = 1.0 / vec2(textureSize(src, 0));

	vec3 sourceColor = textureLod(src, uv, 0).rgb;
	vec3 result = sourceColor * weight[0];

#if HORIZONTAL
	for(int i = 1; i < KERNEL_SIZE; ++i) {
		vec2 delta = vec2(i * invTexSize.x, 0.0);
		vec3 sampleR = textureLod(src, uv + delta, 0).rgb;
		vec3 sampleL = textureLod(src, uv - delta, 0).rgb;
		result += sampleR * weight[i];
		result += sampleL * weight[i];
	}
#else // HORIZONTAL
	for(int i = 1; i < KERNEL_SIZE; ++i) {
		vec2 delta = vec2(0.0, i * invTexSize.y);
		vec3 sampleR = textureLod(src, uv + delta, 0).rgb;
		vec3 sampleL = textureLod(src, uv - delta, 0).rgb;
		result += sampleR * weight[i];
		result += sampleL * weight[i];
	}
#endif // HORIZONTAL

#if ADDITIVE
	vec4 A = texture(additive, uv) * additiveWeight;
	out_color = vec4(result, 0.0) + A;
#else
	out_color = vec4(result, 0.0);
#endif
}
