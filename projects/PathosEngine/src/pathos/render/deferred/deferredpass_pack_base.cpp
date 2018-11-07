#include "deferredpass_pack_base.h"

namespace pathos {

	void MeshDeferredRenderPass_Pack::bindProgram()
	{
		glUseProgram(program);
	}

	MeshDeferredRenderPass_Pack::~MeshDeferredRenderPass_Pack() {
		glDeleteProgram(program);
	}

}