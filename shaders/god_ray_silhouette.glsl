#version 450 core

layout (std140, binding = 1) uniform UBO_Silhouette {
	mat4 modelViewProj;
	vec3 color;
} ubo;

/////////////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

void main() {
	gl_Position = ubo.modelViewProj * vec4(position, 1.0);
}

#endif

/////////////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

out vec4 out_color;

void main() {
	out_color = vec4(ubo.color, 1.0);
}

#endif
