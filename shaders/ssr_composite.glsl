#version 450 core

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D inSSR;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

void main() {
	vec2 screenUV = fs_in.screenUV;
	vec3 ssr = texture(inSSR, screenUV).rgb;
	outSceneColor = vec4(ssr, 0.0);
}
