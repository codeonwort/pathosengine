#include "deferredpass_pack_base.h"
#include "pathos/render/render_device.h"

namespace pathos {

	void MeshDeferredRenderPass_Pack::bindProgram(RenderCommandList& cmdList) {
		cmdList.useProgram(program);
	}

	MeshDeferredRenderPass_Pack::~MeshDeferredRenderPass_Pack() {
		gRenderDevice->deleteProgram(program);
	}

}
