#pragma once

#include <stdint.h>
#include <algorithm>

inline uint32_t to_uint(float r, float g, float b, float a = 1.0) {
	uint32_t R = static_cast<uint32_t>(r * 255.0f);
	uint32_t G = static_cast<uint32_t>(g * 255.0f);
	uint32_t B = static_cast<uint32_t>(b * 255.0f);
	uint32_t A = static_cast<uint32_t>(a * 255.0f);
	// TODO: Switch to std::clamp in C++17
	//       But... is C++17 necessary?
	R = std::max(0u, std::min(0xffu, R));
	G = std::max(0u, std::min(0xffu, G));
	B = std::max(0u, std::min(0xffu, B));
	A = std::max(0u, std::min(0xffu, A));
	return (A << 24) | (R << 16) | (G << 8) | B;
}

inline void to_float3(uint32_t rgb, float output[3]) {
	output[0] = static_cast<float>((rgb & 0x00ff0000) >> 16) / 255.0f;
	output[1] = static_cast<float>((rgb & 0x00ff0000) >> 16) / 255.0f;
	output[2] = static_cast<float>((rgb & 0x00ff0000) >> 16) / 255.0f;
}