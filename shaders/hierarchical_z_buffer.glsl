#version 450 core

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

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

	outDepths = vec2(sceneDepth, sceneDepth);
}

#else // FIRST_MIP

// Downsample HiZ mip[N-1] into HiZ mip[N]
void main() {
	vec2 screenUV = fs_in.screenUV;

	vec4 depths = textureGather(inPrevDepth, screenUV);
	float minDepth = min(min(depths.x, depths.y), min(depths.z, depths.w));
	float maxDepth = max(max(depths.x, depths.y), max(depths.z, depths.w));

	outDepths = vec2(minDepth, maxDepth);
}

#endif // FIRST_MIP
