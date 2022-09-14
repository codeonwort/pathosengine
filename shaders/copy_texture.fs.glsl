#version 460 core

layout (binding = 0) uniform sampler2D tex_source;
layout (location = 0) out vec4 out_color;

in VS_OUT {
	vec2 screenUV;
} fs_in;

void main() {
	out_color = texture(tex_source, fs_in.screenUV);
}
