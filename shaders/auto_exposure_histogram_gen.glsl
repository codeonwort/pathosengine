#version 460 core

// https://www.alextardif.com/HistogramLuminance.html

#include "common.glsl"

// -------------------------------------------------------
// Defines

#define WORKGROUP_SIZE_X      16
#define WORKGROUP_SIZE_Y      16
#define NUM_HISTOGRAM_BINS    256
#define EPSILON               0.01

// --------------------------------------------------------
// Input

layout (std140, binding = 1) uniform UBO {
	uint inputWidth;
	uint inputHeight;
	float minLogLuminance;
	float oneOverLogLuminanceRange;
} ubo;

layout (binding = 0) uniform sampler2D sceneColor;

// --------------------------------------------------------
// Output

layout (std140, binding = 0) buffer BUFFER_Histogram {
	uint bins[];
};

// --------------------------------------------------------
// Shader

layout (local_size_x = WORKGROUP_SIZE_X, local_size_y = WORKGROUP_SIZE_Y) in;

shared uint sharedHistogram[NUM_HISTOGRAM_BINS];

uint getHistogramBinIndex(vec3 color) {
	float luminance = dot(color, vec3(0.2125, 0.7154, 0.0721));
	if (luminance < EPSILON) return 0; // bins[0]
	float logLuminance = (log2(luminance) - ubo.minLogLuminance) * ubo.oneOverLogLuminanceRange;
	logLuminance = clamp(logLuminance, 0.0, 1.0);
	return uint(logLuminance * 254.0 + 1.0); // bins[1] ~ bins[255]
}

void main() {
	uint groupIndex = gl_LocalInvocationIndex;

	sharedHistogram[groupIndex] = 0;

	groupMemoryBarrier();

	uvec2 texel = gl_GlobalInvocationID.xy;
	if (texel.x < ubo.inputWidth && texel.y < ubo.inputHeight) {
		vec3 color = texelFetch(sceneColor, ivec2(texel), 0).rgb;
		uint binIndex = getHistogramBinIndex(color);
		atomicAdd(sharedHistogram[binIndex], 1);
	}

	groupMemoryBarrier();

	atomicAdd(bins[groupIndex], sharedHistogram[groupIndex]);
}
