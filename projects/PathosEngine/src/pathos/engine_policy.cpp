#include "engine_policy.h"

namespace pathos {

	// - Reverse-Z policy should be constant in entire lifetime of the application.
	// - References:
	//   - https://developer.nvidia.com/content/depth-precision-visualized
	//   - https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/
	constexpr EReverseZPolicy getReverseZPolicy() {
		return EReverseZPolicy::Reverse;
	}

}
