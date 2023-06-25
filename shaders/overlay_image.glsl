#version 460 core

#if VERTEX_SHADER
	#define Interpolants out
#elif FRAGMENT_SHADER
	#define Interpolants in
#endif

Interpolants OverlayImageInterpolants {
	vec2 uv;
} interpolants;

layout (std140, binding = 1) uniform UBO_OverlayImage {
	mat4 transform;
} ubo;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

void main() {
	interpolants.uv = uv;
	gl_Position = ubo.transform * vec4(position, 1.0f);
}

#endif // VERTEX_SHADER

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

layout (binding = 0) uniform sampler2D imageTexture;

out vec4 outColor;

void main() {
	outColor = texture(imageTexture, interpolants.uv);
}

#endif // FRAGMENT_SHADER
