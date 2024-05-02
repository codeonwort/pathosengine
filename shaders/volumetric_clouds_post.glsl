#version 460 core

#include "common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} interpolants;

layout (binding = 0) uniform sampler2D volumetricClouds;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

void main() {
	vec2 uv = interpolants.screenUV;

	outSceneColor = textureLod(volumetricClouds, uv, 0);
}
