#version 450 core

#if VERTEX_SHADER
	#define Interpolants out
#elif FRAGMENT_SHADER
	#define Interpolants in
#endif

Interpolants OverlayStandardInterpolants {
	vec2 uv;
} interpolants;

layout (std140, binding = 1) uniform UBO_OverlayStandard {
	mat4 transform;
	vec4 color;
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

out vec4 out_color;

void main() {
	// #todo: texture sampling
	out_color = ubo.color;
}

#endif // FRAGMENT_SHADER
