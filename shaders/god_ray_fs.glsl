#version 460 core

in VS_OUT {
	vec2 screenUV;
} fs_in;

const int  NUM_SAMPLES = 32; // This can't be put into UBO
const vec2 JITTER_OFFSET = vec2(4.0);
const mat4 bayerMatrix = mat4(
	0.0f, 0.5f, 0.125f, 0.625f,
	0.75f, 0.22f, 0.875f, 0.375f,
	0.1875f, 0.6875f, 0.0625f, 0.5625,
	0.9375f, 0.4375f, 0.8125f, 0.3125
);

layout (std140, binding = 1) uniform UBO_GodRay {
	vec2 lightPos;
	float alphaDecay; // default: 0.92
	float density;    // default: 1.1
} ubo;

layout (binding = 0) uniform sampler2D src;

out vec4 out_color;

void main() {
	ivec2 screenXY = ivec2(gl_FragCoord.xy);
	float ditherValue = bayerMatrix[screenXY.x % 4][screenXY.y % 4];

	vec2 uv = fs_in.screenUV;

	vec2 delta = (uv - ubo.lightPos) * (1.0 / (ubo.density * NUM_SAMPLES));
	vec2 pos = uv;
	vec4 result = vec4(0.0);
	float alpha = 1.0;

	vec2 offset = JITTER_OFFSET / textureSize(src, 0);

	for(int i = 0; i < NUM_SAMPLES; ++i) {
		pos -= delta;
		result += alpha * texture2D(src, pos + offset * (0.5 - ditherValue));
		alpha *= ubo.alphaDecay;
	}
	out_color = result;
}
