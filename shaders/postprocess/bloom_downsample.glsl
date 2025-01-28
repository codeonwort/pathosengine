#version 460 core

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

#include "deferred_common.glsl"

layout (binding = 0) uniform sampler2D inputTexture; // sceneColor

layout (location = 0) out vec3 outputColor; // sceneColorDownsample

in VS_OUT {
	vec2 screenUV;
} fs_in;

void main() {
	vec2 uv = fs_in.screenUV;
	vec2 invSize = 1.0 / textureSize(inputTexture, 0);

	// Take 13 samples around current texel:
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = texture(inputTexture, uv + vec2(-2, 2) * invSize).rgb;
	vec3 b = texture(inputTexture, uv + vec2(0, 2) * invSize).rgb;
	vec3 c = texture(inputTexture, uv + vec2(2, 2) * invSize).rgb;

	vec3 d = texture(inputTexture, uv + vec2(-2, 0) * invSize).rgb;
	vec3 e = texture(inputTexture, uv + vec2(0, 0) * invSize).rgb;
	vec3 f = texture(inputTexture, uv + vec2(2, 0) * invSize).rgb;

	vec3 g = texture(inputTexture, uv + vec2(-2, -2) * invSize).rgb;
	vec3 h = texture(inputTexture, uv + vec2(0, -2) * invSize).rgb;
	vec3 i = texture(inputTexture, uv + vec2(2, -2) * invSize).rgb;

	vec3 j = texture(inputTexture, uv + vec2(-1, 1) * invSize).rgb;
	vec3 k = texture(inputTexture, uv + vec2(1, 1) * invSize).rgb;
	vec3 l = texture(inputTexture, uv + vec2(-1, -1) * invSize).rgb;
	vec3 m = texture(inputTexture, uv + vec2(1, -1) * invSize).rgb;

	vec3 result = e * 0.125
		+ (a + c + g + i) * 0.03125
		+ (b + d + f + h) * 0.0625
		+ (j + k + l + m) * 0.125;

	outputColor.rgb = max(result, vec3(0.0001));
}
