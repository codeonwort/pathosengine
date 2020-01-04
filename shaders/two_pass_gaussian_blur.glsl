#version 430 core

// NOTE: In Shader object, define HORIZONTAL as 1 for horizontal blur

layout (binding = 0) uniform sampler2D src;

// Supports 5, 6, 7
#ifndef KERNEL_SIZE
#define KERNEL_SIZE 5 // Actual kernel size is (KERNEL_SIZE * 2 - 1)
#endif

#if KERNEL_SIZE == 5
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
	ivec2 origin = ivec2(gl_FragCoord.xy);
	vec3 result = texelFetch(src, origin, 0).rgb * weight[0];

#if HORIZONTAL
	for(int i = 1; i < KERNEL_SIZE; ++i) {
		result += texelFetch(src, origin + ivec2(1, 0), 0).rgb * weight[i];
		result += texelFetch(src, origin - ivec2(1, 0), 0).rgb * weight[i];
	}
#else
	for(int i = 1; i < KERNEL_SIZE; ++i) {
		result += texelFetch(src, origin + ivec2(0, 1), 0).rgb * weight[i];
		result += texelFetch(src, origin - ivec2(0, 1), 0).rgb * weight[i];
	}
#endif

	out_color = vec4(result, 0.0);
}
