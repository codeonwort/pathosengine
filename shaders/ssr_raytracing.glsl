#version 450 core

#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D inSceneColor;

// --------------------------------------------------------
// Output

layout (location = 0) out vec3 outRayTracingResult;

// --------------------------------------------------------
// Shader

void main() {
	vec2 screenUV = fs_in.screenUV;

	vec3 sceneColor = texture(inSceneColor, screenUV).xyz;

	// Dummy value to test pass-through
	vec3 finalRadiance = sceneColor * 0.5;

	outRayTracingResult = finalRadiance;
}
