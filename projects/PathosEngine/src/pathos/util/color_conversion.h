#pragma once

#include "badger/types/int_types.h"
#include <algorithm>

namespace pathos {

	/// <summary>
	/// float [r, g, b, a] -> uint32 0xaarrggbb.
	/// </summary>
	inline uint32 to_uint32(float r, float g, float b, float a = 1.0) {
		uint32 R = static_cast<uint32>(r * 255.0f);
		uint32 G = static_cast<uint32>(g * 255.0f);
		uint32 B = static_cast<uint32>(b * 255.0f);
		uint32 A = static_cast<uint32>(a * 255.0f);
		R = std::clamp(R, 0u, 0xffu);
		G = std::clamp(G, 0u, 0xffu);
		B = std::clamp(B, 0u, 0xffu);
		A = std::clamp(A, 0u, 0xffu);
		return (A << 24) | (R << 16) | (G << 8) | B;
	}

	/// <summary>
	/// uint32 0xrrggbb -> float [r, g, b].
	/// </summary>
	inline void to_float3(uint32 rgb, float output[3]) {
		output[0] = static_cast<float>((rgb & 0x00ff0000) >> 16) / 255.0f;
		output[1] = static_cast<float>((rgb & 0x0000ff00) >> 8) / 255.0f;
		output[2] = static_cast<float>((rgb & 0x000000ff)) / 255.0f;
	}

	/// <summary>
	/// uint32 0xaarrggbb -> float [r, g, b, a].
	/// </summary>
	inline void to_float4(uint32 argb, float output[4]) {
		output[0] = static_cast<float>((argb & 0x00ff0000) >> 16) / 255.0f;
		output[1] = static_cast<float>((argb & 0x0000ff00) >> 8) / 255.0f;
		output[2] = static_cast<float>((argb & 0x000000ff)) / 255.0f;
		output[3] = static_cast<float>((argb & 0xff000000) >> 24) / 255.0f;
	}

}
