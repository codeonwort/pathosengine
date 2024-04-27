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

layout (location = 0) out float outLogLuminance;

// --------------------------------------------------------
// Shader

void main() {
	vec2 uv = fs_in.screenUV;

	vec3 rgb = textureLod(sceneColor, uv, 0).rgb;
	float sceneLuminance = dot(rgb, vec3(0.2125, 0.7154, 0.0721));

	float eps = 0.05; // #wip: How small it should be? Too small -> too bright when skybox is black
	outLogLuminance = log(max(eps, sceneLuminance));
}
