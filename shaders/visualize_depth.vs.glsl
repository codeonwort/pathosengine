// Visualize scene depth

#version 460 core

layout (location = 0) in vec3 position;

layout (std140, binding = 0) uniform UBO_VisualizeDepth {
	mat4x4 mvp;
	vec4 zRange;
} ubo;

void main() {
	gl_Position = ubo.mvp * vec4(position, 1.0f);
}
