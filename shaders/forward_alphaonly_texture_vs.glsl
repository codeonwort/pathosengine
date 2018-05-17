#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out VS_OUT {
    vec2 uv;
} vs_out;

uniform mat4 mvpTransform;

void main() {
    vs_out.uv = uv;
    gl_Position = mvpTransform * vec4(position, 1.0);
}