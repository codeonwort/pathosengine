#version 430 core

layout (location = 0) in vec3 position;

void main() {
	const vec3[4] vertices = vec3[4](vec3(-1,-1,1), vec3(1,-1,1), vec3(-1,1,1), vec3(1,1,1));
	gl_Position = vec4(vertices[gl_VertexID], 1);
}