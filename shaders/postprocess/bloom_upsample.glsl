#version 460 core

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (binding = 0) uniform sampler2D inputTexture; // prevDownsampleMip

layout (std140, binding = 1) uniform UBO_BloomUpsample {
	float filterRadius;
} ubo;

// --------------------------------------------------------
// Output

layout (location = 0) out vec3 outColor; // currentDownsampleMip or sceneBloom

// --------------------------------------------------------
// Shader

void main() {
	vec2 uv = fs_in.screenUV;
	vec2 invSize = ubo.filterRadius / textureSize(inputTexture, 0);

	// Take 9 samples around current texel:
	// a - b - c
	// d - e - f
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = texture(inputTexture, uv + vec2(-1, 1) * invSize).rgb;
	vec3 b = texture(inputTexture, uv + vec2(0, 1) * invSize).rgb;
	vec3 c = texture(inputTexture, uv + vec2(1, 1) * invSize).rgb;

	vec3 d = texture(inputTexture, uv + vec2(-1, 0) * invSize).rgb;
	vec3 e = texture(inputTexture, uv + vec2(0, 0) * invSize).rgb;
	vec3 f = texture(inputTexture, uv + vec2(1, 0) * invSize).rgb;

	vec3 g = texture(inputTexture, uv + vec2(-1, -1) * invSize).rgb;
	vec3 h = texture(inputTexture, uv + vec2(0, -1) * invSize).rgb;
	vec3 i = texture(inputTexture, uv + vec2(1, -1) * invSize).rgb;

	outColor = (e * 4.0 + (b + d + f + h) * 2.0 + (a + c + g + i)) / 16.0;
}
