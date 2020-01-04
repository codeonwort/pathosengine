#version 430 core

layout (location = 0) in vec3 position;

out VS_OUT {
	vec2 uv;
} vs_out;

void main() {
	const vec3[4] vertices = vec3[4](vec3(-1,-1,1), vec3(1,-1,1), vec3(-1,1,1), vec3(1,1,1));
	const vec2[4] texcoords = vec2[4](vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1));

	vs_out.uv = texcoords[gl_VertexID];

	gl_Position = vec4(vertices[gl_VertexID], 1);
}
