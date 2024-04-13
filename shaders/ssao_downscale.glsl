#version 460 core

#include "deferred_common.glsl"

// -------------------------------------------------------
// Defines

#define AVERAGE_SAMPLES 0

// 0: viewPosZ, 1: sceneDepth
#define OUTPUT_SCENE_DEPTH 1

// -------------------------------------------------------
// Shader Resources

layout (local_size_x = 64) in;

layout (binding = 0) uniform sampler2D sceneDepth;
layout (binding = 1, rgba32ui) readonly uniform uimage2D gbufferA;
layout (binding = 2, rgba32f) readonly uniform image2D gbufferB;
layout (binding = 3, rgba16f) writeonly uniform image2D outHalfNormalAndDepth;

// -------------------------------------------------------

// Output normalVS and viewPosZ in half resolution.
void main() {
	ivec2 sceneSize = imageSize(gbufferA);
	if (gl_GlobalInvocationID.x >= sceneSize.x) {
		return;
	}

	ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);

#if AVERAGE_SAMPLES
	float maxDepth = -1.0 / 0.0;
	vec3 averageNormal = vec3(0.0); // in view space

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			ivec2 fullResTexel = currentTexel * 2 + ivec2(j, i);
			vec2 uv = vec2(fullResTexel) / vec2(sceneSize);

#if OUTPUT_SCENE_DEPTH
			float currentDepth = texture(sceneDepth, uv).x;
#else
			// My gbuffers store view-space values, not world-space ones.
			// No need to reconstruct view-space position from scene depth.
			float currentDepth = -imageLoad(gbufferB, fullResTexel).z;
#endif
			maxDepth = max(maxDepth, currentDepth);

			// NOTE: Should match with unpackGBuffer() in deferred_common.glsl
			uvec4 gbuffer = imageLoad(gbufferA, fullResTexel);
			vec2 temp = unpackHalf2x16(gbuffer.y);
			vec2 temp2 = unpackHalf2x16(gbuffer.z);
			vec3 n = normalize(vec3(temp.y, temp2.x, temp2.y));
			averageNormal += n;
		}
	}

	// #todo-ssao: Oops. Normalizing a zero vector results in NaN.
	if (length(averageNormal) > 1e-6) {
		averageNormal = normalize(averageNormal);
	} else {
		averageNormal = vec3(0.0, 0.0, 1.0);
	}

	vec4 nd = vec4( averageNormal, maxDepth );
#else // AVERAGE_SAMPLES
	ivec2 fullResTexel = currentTexel * 2;

#if OUTPUT_SCENE_DEPTH
	vec2 uv = vec2(fullResTexel) / vec2(sceneSize);
	float d = texture(sceneDepth, uv).x;
#else
	// gbufferB.z = view space Z position
	float d = -imageLoad(gbufferB, fullResTexel).z;
#endif

	uvec4 gbuffer = imageLoad(gbufferA, fullResTexel);
	vec2 temp = unpackHalf2x16(gbuffer.y); // normalVS.x
	vec2 temp2 = unpackHalf2x16(gbuffer.z); // normalVS.y, normalVS.z
	vec3 n = normalize(vec3(temp.y, temp2.x, temp2.y));

	vec4 nd = vec4(n, d);
#endif // AVERAGE_SAMPLES

	imageStore(outHalfNormalAndDepth, currentTexel, nd);
}
