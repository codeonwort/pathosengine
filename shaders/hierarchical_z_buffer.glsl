#version 450 core

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D inPrevDepth;

// --------------------------------------------------------
// Shader

void main() {
	vec2 screenUV = fs_in.screenUV;

	vec4 depths = textureGather(inPrevDepth, screenUV);
	gl_FragDepth = max(max(depths.x, depths.y), max(depths.z, depths.w));
}
