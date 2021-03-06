#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

uniform mat4 transform;

out VS_OUT {
	vec2 uv;
} vs_out;

void main() {
	vs_out.uv = uv;
	gl_Position = transform * vec4(position, 1.0f);
}
