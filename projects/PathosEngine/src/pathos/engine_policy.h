#pragma once

// WARNING: Minimize includes!!!
//          Never include other than primitive types.
#include <inttypes.h>

// Configurations that globally affect the engine.
// Separated here to minimize #include dependencies, but is this too random?

namespace pathos {

	// - Reverse-Z policy should be constant in entire lifetime of the application.
	// - References:
	//   - https://developer.nvidia.com/content/depth-precision-visualized
	//   - https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/
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
