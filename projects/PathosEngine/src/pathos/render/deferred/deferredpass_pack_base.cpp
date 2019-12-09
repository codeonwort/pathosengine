#include "deferredpass_pack_base.h"

namespace pathos {

	void MeshDeferredRenderPass_Pack::bindProgram(RenderCommandList& cmdList) {
		cmdList.useProgram(program);
	}

	MeshDeferredRenderPass_Pack::~MeshDeferredRenderPass_Pack() {
		glDeleteProgram(program);
	}

}
