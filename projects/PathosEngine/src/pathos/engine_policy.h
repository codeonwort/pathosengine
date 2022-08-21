#pragma once

// WARNING: Minimize includes!!!
//          Never include other than primitive types.
#include <inttypes.h>

// Configurations that globally affect the engine.
// Separated here to minimize #include dependencies, but is this too random?

namespace pathos {

	enum class EReverseZPolicy : uint8_t {
		Traditional,
		Reverse
	};
	extern constexpr EReverseZPolicy getReverseZPolicy();

}
