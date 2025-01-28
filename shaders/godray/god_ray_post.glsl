#version 460 core

#include "core/common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} interpolants;

layout (binding = 0) uniform sampler2D godRay;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

void main() {
	vec2 uv = interpolants.screenUV;

	outSceneColor = textureLod(godRay, uv, 0);
}
