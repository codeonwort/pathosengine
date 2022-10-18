#version 460 core

#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_TAA {
	float historyWeight;
} ubo;

layout (binding = 0) uniform sampler2D inSceneColor;
layout (binding = 1) uniform sampler2D inSceneColorHistory;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

void main() {
	vec2 screenUV = fs_in.screenUV;
	vec4 curr = texture(inSceneColor, screenUV);
	vec4 prev = texture(inSceneColorHistory, screenUV);
	float w = ubo.historyWeight;

	outSceneColor = (1.0 - w) * curr + w * prev;
}
