#version 430 core

#include "deferred_common.glsl"

layout (local_size_x = 64) in;

//shared float shared_depths[gl_WorkGroupSize.x];

layout (binding = 0, rgba32f) readonly uniform image2D halfNormalAndDepth;
layout (binding = 1, r32f) writeonly uniform image2D ssaoMap;

layout (std140, binding = 1) uniform UBO_SSAO {
	float ssaoRadius;
	bool enable;
	uint pointCount;
	bool randomizePoints;
} uboSSAO;

layout (std140, binding = 2) uniform UBO_RANDOM {
	vec4 points[256];
	vec4 randomVectors[256];
} uboRandom;

float computeAO(ivec2 texel, vec2 uv) {
	if (!uboSSAO.enable) {
		return 1.0;
	}

	vec4 normalAndDepth = imageLoad(halfNormalAndDepth, texel.xy);
	vec3 currentNormal = normalAndDepth.xyz;
	float currentDepth = normalAndDepth.w;

	int i, j, n;
	float occ = 0.0;
	float total = 0.0;

	n = (int(float(texel.x) * 7123.2315 + 125.232)
		* int(float(texel.y) * 3137.1519 + 234.8))
		^ int(currentDepth);
	vec4 v = uboRandom.randomVectors[n & 255];

	float r = (v.x + 3.0) * 0.1; // radius randomizer
	if (!uboSSAO.randomizePoints) {
		r = 0.5;
	}

	for (i = 0; i < uboSSAO.pointCount; i++) {
		vec3 dir = uboRandom.points[i].xyz;
		if (dot(currentNormal, dir) < 0.0) {
			dir = -dir;
		}

		float f = 0.0;          // distance we've stepped in this direction
		float z = currentDepth; // interpolated depth

		total += 4.0; // 4 steps; #todo-ssao: Make configurable

		for (j = 0; j < 4; j++) {
			f += r;
			z -= dir.z * f;

			vec2 neighborUV = uv + dir.xy * f * uboSSAO.ssaoRadius;
			ivec2 neighborTexel = ivec2(neighborUV * imageSize(ssaoMap).xy);
			float neighborDepth = imageLoad(halfNormalAndDepth, neighborTexel).w;

			float d = neighborDepth - currentDepth;
			d *= d;

			if ((z - neighborDepth) > 0.0) {
				occ += 4.0 / (1.0 + d);
			}
		}
	}

	float aoAmount = max(0.0, 1.0 - occ / total);
	return mix(0.2, 1.0, aoAmount);
}

void main() {
	ivec2 sceneSize = imageSize(ssaoMap);
	if (gl_GlobalInvocationID.x >= sceneSize.x) {
		return;
	}
	
	ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2(currentTexel) / vec2(sceneSize);
	vec2 ndcXY = vec2(2.0 * uv.x - 1.0, 1.0 - 2.0 * uv.y);

	float ao = computeAO(currentTexel, uv);
	imageStore(ssaoMap, currentTexel, vec4(ao));
}
