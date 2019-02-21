// Visualize scene depth

#version 460 core

layout (location = 0) out vec4 color;

layout (std140, binding = 0) uniform UBO_VisualizeDepth {
	mat4x4 mvp;
	vec4 zRange;
} ubo;

void main() {
	float n = ubo.zRange.x;
	float f = ubo.zRange.y;
	float z = (2 * n) / (f + n - gl_FragCoord.z * (f - n));
	color = vec4(z, z, z, 1.0);
}
