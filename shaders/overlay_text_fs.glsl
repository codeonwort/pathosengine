#version 430 core

uniform vec3 color;
uniform sampler2D texSampler;

in VS_OUT {
    vec2 uv;
} fs_in;

out vec4 out_color;

void main() {
    float alpha = texture2D(texSampler, fs_in.uv).r;
    out_color = vec4(color, alpha);
}