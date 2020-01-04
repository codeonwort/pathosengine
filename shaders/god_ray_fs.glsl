#version 450 core

in VS_OUT {
	vec2 uv;
} fs_in;

layout (binding = 0) uniform sampler2D src;

layout (location = 0) uniform vec2 lightPos = vec2(0.5, 0.5);

const float alphaDecay = 0.95;
const float density = 1.1;
const int NUM_SAMPLES = 100;

out vec4 out_color;

void main() {
	vec2 uv = fs_in.uv;
	vec2 delta = (uv - lightPos) * (1.0 / (density * NUM_SAMPLES));
	vec2 pos = uv;
	vec4 result = vec4(0.0);
	float alpha = 1.0;

	for(int i = 0; i < NUM_SAMPLES; ++i) {
		pos -= delta;
		result += alpha * texture2D(src, pos);
		alpha *= alphaDecay;
	}
	out_color = result;
}
