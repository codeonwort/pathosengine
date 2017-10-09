#include "overlaypass_base.h"

namespace pathos {

	OverlayPass::~OverlayPass() {
		if (program) {
			glDeleteProgram(program);
			program = 0;
		}
	}

}