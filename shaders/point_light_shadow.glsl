#version 450 core

layout (std140, binding = 1) uniform UBO_PointLightShadow {
	mat4 model;
	mat4 viewproj;
} ubo;

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

void main() {
	gl_Position = ubo.viewproj * ubo.model * vec4(position, 1.0f);
}

#endif

////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

out vec4 color;

void main() {
	color = vec4(gl_FragCoord.z, 0.0f, 0.0f, 1.0f);
}

#endif
