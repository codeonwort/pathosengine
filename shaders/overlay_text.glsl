#version 460 core

#if VERTEX_SHADER
	#define Interpolants out
#elif FRAGMENT_SHADER
	#define Interpolants in
#endif

Interpolants OverlayTextInterpolants {
	vec2 uv;
} interpolants;

layout (std140, binding = 1) uniform UBO_OverlayText {
	mat4 transform;
	vec4 color;
} ubo;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

void main() {
    interpolants.uv = uv;
    gl_Position = ubo.transform * vec4(position, 1.0);
}

#endif // VERTEX_SHADER

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

uniform sampler2D texSampler;

out vec4 out_color;

void main() {
    float alpha = texture2D(texSampler, interpolants.uv).r;
    out_color = vec4(ubo.color.xyz, alpha);
}

#endif // FRAGMENT_SHADER
