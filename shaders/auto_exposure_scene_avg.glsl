#version 460 core

#include "common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (location = 1) uniform float minLogLuminance;

layout (binding = 0) uniform sampler2D sceneColor;

// --------------------------------------------------------
// Output

layout (location = 0) out float outLogLuminance;

// --------------------------------------------------------
// Shader

void main() {
	vec2 uv = fs_in.screenUV;

	vec3 rgb = textureLod(sceneColor, uv, 0).rgb;
	float sceneLuminance = dot(rgb, vec3(0.2125, 0.7154, 0.0721));

	outLogLuminance = max(minLogLuminance, log(sceneLuminance));
}
