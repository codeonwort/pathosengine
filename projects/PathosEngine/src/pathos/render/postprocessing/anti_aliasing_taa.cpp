#include "anti_aliasing_taa.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/util/engine_util.h"

namespace pathos {

	struct UBO_TAA {
		static constexpr uint32 BINDING_POINT = 1;
		
		vector4 dummy;
	};

}

namespace pathos {

	void TAA::initializeResources(RenderCommandList& cmdList) {
		ubo.init<UBO_TAA>("UBO_FXAA");

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_TAA");
	}

	void TAA::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		markDestroyed();
	}

	void TAA::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(TAA);

		// #todo-taa
	}

}
