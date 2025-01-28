#version 460 core

// https://www.alextardif.com/HistogramLuminance.html

#include "core/common.glsl"

// -------------------------------------------------------
// Defines

#define WORKGROUP_SIZE_X      16
#define WORKGROUP_SIZE_Y      16
#define NUM_HISTOGRAM_BINS    256

// --------------------------------------------------------
// Input

layout (std140, binding = 1) uniform UBO {
	uint pixelCount;
	float minLogLuminance;
	float logLuminanceRange;
	float timeDelta;
	float tau; // ex) 1.1
} ubo;

layout (std430, binding = 0) readonly buffer BUFFER_Histogram {
	uint bins[];
};

// --------------------------------------------------------
// Output

layout (binding = 0, r32f) uniform image2D rwLuminance;

// --------------------------------------------------------
// Shader

layout (local_size_x = WORKGROUP_SIZE_X, local_size_y = WORKGROUP_SIZE_Y) in;

shared float sharedHistogram[NUM_HISTOGRAM_BINS];

void main() {
	uint groupIndex = gl_LocalInvocationIndex; // 0 ~ 255

	float countForBin = float(bins[groupIndex]);
	sharedHistogram[groupIndex] = countForBin * float(groupIndex); // weight * luminanceLevel

	// Maybe I don't understand memory barrier :o
#if 0
	// No memoryBarrierShared() is OK? but to be sure...
	memoryBarrierBuffer();
#else
	memoryBarrierBuffer();
	memoryBarrierShared();
#endif

	for (uint sampleIx = (NUM_HISTOGRAM_BINS >> 1); sampleIx > 0; sampleIx >>= 1) {
		if (groupIndex < sampleIx) {
			sharedHistogram[groupIndex] += sharedHistogram[groupIndex + sampleIx];
		}
	}

	memoryBarrierShared();

	if (groupIndex == 0) {
		float numValidPixels = max(float(ubo.pixelCount) - countForBin, 1.0); // non-black pixel count
		float weightedLogAverage = (sharedHistogram[0] / numValidPixels) - 1.0;
		float weightedAverageLuminance = exp2(((weightedLogAverage / 254.0) * ubo.logLuminanceRange) + ubo.minLogLuminance);
		float luminanceLastFrame = imageLoad(rwLuminance, ivec2(0, 0)).r;
		//float adaptedLuminance = luminanceLastFrame + (weightedAverageLuminance - luminanceLastFrame) * (1.0 - exp(-ubo.timeDelta * ubo.tau));
		float adaptedLuminance = mix(luminanceLastFrame, weightedAverageLuminance, 1.0 - exp(-ubo.timeDelta * ubo.tau));
		imageStore(rwLuminance, ivec2(0, 0), vec4(adaptedLuminance));
	}
}
