#version 430 core

layout (location = 0) in vec3 uv;

uniform vec4 color;
uniform sampler2D tex_sampler;

in VS_OUT {
	vec2 uv;
} fs_in;

out vec4 out_color;

void main() {
	//out_color = texture(tex_sampler, fs_in.uv) + color;
	out_color = color;
}