#pragma once

// Configurations that globally affect the engine.
// These policies should be constant in entire lifetime of the application.
// Separated here to minimize #include dependencies.

// WARNING: Minimize includes!!! Never include other than primitive types.
#include <inttypes.h>

// References:
// 
// [Reverse-Z]
// https://developer.nvidia.com/content/depth-precision-visualized
// https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/
//
// [Units of measurement]
// https://seblagarde.wordpress.com/2015/07/14/siggraph-2014-moving-frostbite-to-physically-based-rendering/

namespace pathos {

	enum class EReverseZPolicy : uint8_t {
		Traditional = 0,
		Reverse = 1
	};
	constexpr EReverseZPolicy getReverseZPolicy() {
		return EReverseZPolicy::Reverse;
	}
	constexpr float getDeviceFarDepth() {
		return (getReverseZPolicy() == EReverseZPolicy::Reverse) ? 0.0f : 1.0f;
	}
	constexpr float getDeviceNearDepth() {
		return (getReverseZPolicy() == EReverseZPolicy::Reverse) ? 1.0f : 0.0f;
	}

}
