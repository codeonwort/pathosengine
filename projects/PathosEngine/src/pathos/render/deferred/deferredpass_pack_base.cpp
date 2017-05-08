#include "deferredpass_pack_base.h"

namespace pathos {

	MeshDeferredRenderPass_Pack::~MeshDeferredRenderPass_Pack() {
		glDeleteProgram(program);
	}

}