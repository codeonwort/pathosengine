#version 450 core

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D src;

layout (location = 0) uniform vec2 lightPos = vec2(0.5, 0.5);

const float alphaDecay = 0.92;
const float density = 1.1;
const int NUM_SAMPLES = 32;

out vec4 out_color;

void main() {
	const mat4 bayerMatrix = mat4(
		0.0f, 0.5f, 0.125f, 0.625f,
		0.75f, 0.22f, 0.875f, 0.375f,
		0.1875f, 0.6875f, 0.0625f, 0.5625,
		0.9375f, 0.4375f, 0.8125f, 0.3125
	);

	ivec2 screenXY = ivec2(gl_FragCoord.xy);
	float ditherValue = bayerMatrix[screenXY.x % 4][screenXY.y % 4];

	vec2 uv = fs_in.screenUV;

	vec2 delta = (uv - lightPos) * (1.0 / (density * NUM_SAMPLES));
	vec2 pos = uv;
	vec4 result = vec4(0.0);
	float alpha = 1.0;

	// #todo-godray: Better offset
	//vec2 offset = vec2(0.01, 0.01);
	vec2 offset = vec2(4.0) / textureSize(src, 0);

	for(int i = 0; i < NUM_SAMPLES; ++i) {
		pos -= delta;
		result += alpha * texture2D(src, pos + offset * (0.5 - ditherValue));
		alpha *= alphaDecay;
	}
	out_color = result;
}
