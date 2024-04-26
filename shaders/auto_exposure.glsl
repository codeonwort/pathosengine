#version 460 core

// https://bruop.github.io/exposure/

#include "common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D sceneColor;

// --------------------------------------------------------
// Output

layout (location = 0) out float outLuminance;

// --------------------------------------------------------
// Shader

void main() {
	vec2 uv = fs_in.screenUV;

	vec3 rgb = textureLod(sceneColor, uv, 0).rgb;
	float sceneLuminance = dot(rgb, vec3(0.2125, 0.7154, 0.0721));

	sceneLuminance = min(65536.0, max(0.0, sceneLuminance)); // Clamp to 16-bit range

	outLuminance = sceneLuminance;
}
