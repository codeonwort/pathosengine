#version 460 core

layout (std140, binding = 1) uniform UBO_CSM {
	mat4 depthMVP;
} ubo;

////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

void main() {
	gl_Position = ubo.depthMVP * vec4(position, 1.0);
}

#endif

////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

void main() {
}

#endif
