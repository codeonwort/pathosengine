// geometry shader for NormalRenderer

#version 430 core

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
	vec3 normal;
	vec4 normalAdded;
} gs_in[];

void main() {
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gs_in[0].normalAdded;
	EmitVertex();
	EndPrimitive();
}