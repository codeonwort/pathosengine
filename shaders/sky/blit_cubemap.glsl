#version 460 core

layout (location = 0) uniform mat4 transform;
layout (location = 1) uniform float inputMip;

layout (binding = 0) uniform samplerCube sourceCube;

// ------------------------------------------------------

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

out vec3 localDir;

void main() {
    //localDir = (transform * vec4(normalize(position), 0)).xyz;
    localDir = normalize(position);
    gl_Position = transform * vec4(position, 1.0);
}

#endif // VERTEX_SHADER

// ------------------------------------------------------

#if FRAGMENT_SHADER

in vec3 localDir;
out vec4 outColor;

void main() {
    outColor = textureLod(sourceCube, localDir, inputMip);
}

#endif // FRAGMENT_SHADER
