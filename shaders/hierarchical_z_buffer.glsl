#version 460 core

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_HiZ {
	uvec2 prevSize;
	uvec2 currentSize;
	uint needsExtraSampleX;
	uint needsExtraSampleY;
} ubo;

layout (binding = 0) uniform sampler2D inPrevDepth;

// --------------------------------------------------------
// Output

layout (location = 0) out vec2 outDepths; // (min, max)

// --------------------------------------------------------
// Shader

#if FIRST_MIP

// Copy sceneDepth to sceneDepthHiZ mip[0]
void main() {
	vec2 screenUV = fs_in.screenUV;

	float sceneDepth = texelFetch(inPrevDepth, ivec2(gl_FragCoord.xy), 0).r;

#if REVERSE_Z
	sceneDepth = 1.0 - sceneDepth;
#endif

	outDepths = vec2(sceneDepth, sceneDepth);
}

#else // FIRST_MIP

// Downsample HiZ mip[N-1] into HiZ mip[N]
void main() {
	vec2 screenUV = fs_in.screenUV;
	ivec2 currentTexel = ivec2(ubo.currentSize * screenUV);
	ivec2 prevTexel = currentTexel * 2;

#if 0
	vec4 depths = textureGather(inPrevDepth, screenUV);
#else
	vec4 depths;
	depths.x = texelFetch(inPrevDepth, prevTexel + ivec2(0, 0), 0).r;
	depths.y = texelFetch(inPrevDepth, prevTexel + ivec2(1, 0), 0).r;
	depths.z = texelFetch(inPrevDepth, prevTexel + ivec2(0, 1), 0).r;
	depths.w = texelFetch(inPrevDepth, prevTexel + ivec2(1, 1), 0).r;
#endif
	float minDepth = min(min(depths.x, depths.y), min(depths.z, depths.w));
	float maxDepth = max(max(depths.x, depths.y), max(depths.z, depths.w));

	if (ubo.needsExtraSampleX != 0) {
		vec2 extraDepths;
		extraDepths.x = texelFetch(inPrevDepth, prevTexel + ivec2(2, 0), 0).r;
		extraDepths.y = texelFetch(inPrevDepth, prevTexel + ivec2(2, 1), 0).r;
		minDepth = min(minDepth, min(extraDepths.x, extraDepths.y));
		maxDepth = max(maxDepth, max(extraDepths.x, extraDepths.y));
	}
	if (ubo.needsExtraSampleY != 0) {
		vec2 extraDepths;
		extraDepths.x = texelFetch(inPrevDepth, prevTexel + ivec2(0, 2), 0).r;
		extraDepths.y = texelFetch(inPrevDepth, prevTexel + ivec2(1, 2), 0).r;
		minDepth = min(minDepth, min(extraDepths.x, extraDepths.y));
		maxDepth = max(maxDepth, max(extraDepths.x, extraDepths.y));
	}
	if (ubo.needsExtraSampleX != 0 && ubo.needsExtraSampleY != 0) {
		float extraDepth = texelFetch(inPrevDepth, prevTexel + ivec2(2, 2), 0).r;
		minDepth = min(minDepth, extraDepth);
		maxDepth = max(maxDepth, extraDepth);
	}

	outDepths = vec2(minDepth, maxDepth);
}

#endif // FIRST_MIP
