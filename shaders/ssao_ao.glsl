#version 450 core

#include "deferred_common.glsl"

// -------------------------------------------------------
// Defines

#define PI                         3.14159265359
#define TWO_PI                     6.28318530718
#define HALF_PI                    1.57079632679489661923

#define WORKGROUP_SIZE_X 16
#define WORKGROUP_SIZE_Y 16

#ifndef SSAO_MAX_SAMPLE_POINTS
	#define SSAO_MAX_SAMPLE_POINTS 64
#endif
#ifndef SSAO_NUM_ROTATION_NOISE
	#define SSAO_NUM_ROTATION_NOISE 16
#endif

// Enable if LDS is cheaper than imageLoad. (heard that some GPUs can't benefit from it)
#define USE_SHARED_SAMPLES 1

// GPU Zen 1. Robust Screen Space Ambient Occlusion in 1 ms in 1080p on PS4
#define SHADOW_WARRIOR_SSAO 1

// -------------------------------------------------------
// Shader Resources

layout (local_size_x = WORKGROUP_SIZE_X, local_size_y = WORKGROUP_SIZE_Y) in;

#if USE_SHARED_SAMPLES
shared vec4 shared_samples[gl_WorkGroupSize.x * gl_WorkGroupSize.y];
#endif

layout (binding = 0, rgba16f) readonly uniform image2D halfNormalAndDepth;
layout (binding = 1, r16f) writeonly uniform image2D ssaoMap;

layout (std140, binding = 1) uniform UBO_SSAO {
	float ssaoRadius; // SSAO radius in world space
	bool enable;
	uint sampleCount;
	bool randomizePoints;
} uboSSAO;

layout (std140, binding = 2) uniform UBO_RANDOM {
	vec4 samplePoints[SSAO_MAX_SAMPLE_POINTS];
	vec4 randomRotations[SSAO_NUM_ROTATION_NOISE];
} uboRandom;

// -------------------------------------------------------

// interleaved gradient noise
float noise_ig(ivec2 texel)
{
	const vec3 m = vec3(0.06711056, 0.0233486, 52.9829189);
	float theta = fract(m.z * fract(dot(vec2(texel), m.xy)));
	return theta;
}

vec2 VogelDiskOffset(int i, float phi)
{
	float r = sqrt(float(i) + 0.5) / sqrt(uboSSAO.sampleCount);
	float theta = 2.4 * float(i) + phi;
	float x = r * cos(theta);
	float y = r * sin(theta);
	return vec2(x, y);
}

vec3 getRandomRotation(ivec2 texel) {
	int ix = (texel.y & 4) << 2 + (texel.x & 4);
	return uboRandom.randomRotations[ix].xyz;
}

float computeAO(ivec2 texel, vec2 uv) {
	if (!uboSSAO.enable) {
		return 1.0;
	}

#if USE_SHARED_SAMPLES
	vec4 normalAndDepth = shared_samples[gl_LocalInvocationIndex];
#else
	vec4 normalAndDepth = imageLoad(halfNormalAndDepth, texel.xy);
#endif
	vec3 currentNormal = normalAndDepth.xyz;
	float currentDepth = normalAndDepth.w;
	vec3 currentPosVS = getViewPositionFromSceneDepth(uv, currentDepth);

#if !SHADOW_WARRIOR_SSAO
	vec3 randomRot = getRandomRotation(texel);
	vec3 tangent = normalize(randomRot - currentNormal * dot(randomRot, currentNormal));
	vec3 bitangent = cross(currentNormal, tangent);
	mat3 TBN = mat3(tangent, bitangent, currentNormal);

	float occlusion = 0.0;

	for (int i = 0; i < uboSSAO.sampleCount; i++) {
		vec3 samplePos = TBN * uboRandom.samplePoints[i].xyz;
		samplePos = currentPosVS + samplePos * uboSSAO.ssaoRadius;

		vec4 offset = vec4(samplePos, 1.0);
		offset = uboPerFrame.projTransform * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		vec2 neighborUV = offset.xy;
		ivec2 neighborTexel = ivec2(neighborUV * imageSize(ssaoMap).xy);

#if USE_SHARED_SAMPLES
		float neighborDepth;
		ivec2 neighborID = ivec2(gl_LocalInvocationID.xy) + neighborTexel - texel;
		if (0 <= neighborID.x && neighborID.x < gl_WorkGroupSize.x && 0 <= neighborID.y && neighborID.y < gl_WorkGroupSize.y) {
			int neighborLocalInvocationIndex = neighborID.y * int(gl_WorkGroupSize.x) + neighborID.x;
			neighborDepth = shared_samples[neighborLocalInvocationIndex].w;
		} else {
			neighborDepth = imageLoad(halfNormalAndDepth, neighborTexel).w;
		}
#else
		float neighborDepth = imageLoad(halfNormalAndDepth, neighborTexel).w;
#endif
		float neighborZ = getViewPositionFromSceneDepth(neighborUV, neighborDepth).z;
			
		const float bias = 0.025;
		float rangeCheck = smoothstep(0.0, 1.0, uboSSAO.ssaoRadius / abs(currentPosVS.z - neighborZ));
		occlusion += (neighborZ >= currentPosVS.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	float finalAO = 1.0 - (occlusion / uboSSAO.sampleCount);
#endif

#if SHADOW_WARRIOR_SSAO
	float ao = 0.0;
	float vdNoise = noise_ig(texel); // per-texel random number for Vogel disk
	
	// Samples generated by Vogel are defined on the unit circle, so need to be scaled down.
	vec2 radius_screen = vec2(uboSSAO.ssaoRadius / currentPosVS.z);
	// #todo: Add ssaoMaxScreenRadius parameter
	//radius_screen = min(radius_screen, uboSSAO.ssaoMaxScreenRadius);
	radius_screen.y *= getAspectRatio();

	for (int i = 0; i < uboSSAO.sampleCount; ++i)
	{
		vec2 sampleOffset = VogelDiskOffset(i, TWO_PI * vdNoise);
		vec2 neighborUV = uv + radius_screen * sampleOffset;

		ivec2 inputTextureSize = imageSize(halfNormalAndDepth);
		ivec2 neighborTexel = ivec2(neighborUV * inputTextureSize);
		float neighborDepth = imageLoad(halfNormalAndDepth, neighborTexel).w;
		vec3 samplePositionVS = getViewPositionFromSceneDepth(neighborUV, neighborDepth);

		vec3 v = samplePositionVS - currentPosVS;

		ao += max(0.0, dot(v, currentNormal) + 0.002 * currentPosVS.z) / (dot(v, v) + 0.001);
	}

	ao = clamp(ao / uboSSAO.sampleCount, 0.0, 1.0);
	ao = 1.0 - ao;
	// #todo: Add contrast parameter
	//ao = pow(ao, uboSSAO.contrast);

	float finalAO = ao;
#endif

	return finalAO;
}

void main() {
	ivec2 sceneSize = imageSize(ssaoMap);
	if (gl_GlobalInvocationID.x >= sceneSize.x || gl_GlobalInvocationID.y >= sceneSize.y) {
		return;
	}
	
	ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2(currentTexel) / vec2(sceneSize);
	vec2 ndcXY = vec2(2.0 * uv.x - 1.0, 1.0 - 2.0 * uv.y);

#if USE_SHARED_SAMPLES
	// Init shared variable
	shared_samples[gl_LocalInvocationIndex] = imageLoad(halfNormalAndDepth, currentTexel.xy);
	barrier();

	// memoryBarrierShared is not necessary according to
	// https://stackoverflow.com/questions/39393560/glsl-memorybarriershared-usefulness
	// memoryBarrierShared();
#endif

	float ao = computeAO(currentTexel, uv);

	imageStore(ssaoMap, currentTexel, vec4(ao));
}
