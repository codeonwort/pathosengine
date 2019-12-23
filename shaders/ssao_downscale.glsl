#version 430 core

#include "deferred_common.glsl"

layout (local_size_x = 64) in;

layout (binding = 0) uniform sampler2D sceneDepth;
layout (binding = 1, rgba32ui) readonly uniform uimage2D gbufferA;
layout (binding = 2, rgba32f) writeonly uniform image2D outHalfNormalAndDepth;

float linearDepth(float depth) {
	float n = uboPerFrame.zRange.x;
	float f = uboPerFrame.zRange.y;
	float linear = (2 * n) / (f + n - depth * (f - n));
	return linear;
}

void main() {
	ivec2 sceneSize = imageSize(gbufferA);
	if (gl_GlobalInvocationID.x >= sceneSize.x) {
		return;
	}

	ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);

	float minDepth = 1.0;
	vec3 averageNormal = vec3(0.0); // in view space

	for (int i=0; i<2; i++) {
		for (int j=0; j<2; j++) {
			ivec2 fullResTexel = currentTexel * 2 + ivec2(j, i);
			vec2 uv = vec2(fullResTexel) / vec2(sceneSize);

			float currentDepth = texture2D(sceneDepth, uv).x;
			minDepth = min(minDepth, linearDepth(currentDepth));

			uvec4 gbuffer = imageLoad(gbufferA, fullResTexel);
			vec2 temp = unpackHalf2x16(gbuffer.y);
			vec2 temp2 = unpackHalf2x16(gbuffer.z);
			vec3 n = normalize(vec3(temp.y, temp2.x, temp2.y));
			averageNormal += n;
		}
	}

	vec4 nd = vec4( normalize(averageNormal), minDepth );
	imageStore(outHalfNormalAndDepth, currentTexel, nd);
}
