#include "post_process.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	PostProcess::~PostProcess() {
		// #todo-scene-capture: It fails on close window.
		// See DeferredRenderer::internal_destroyGlobalResources() for the reason :(
		CHECKF(resourcesDestroyed, "Child classes should override releaseResources() and set this true at the end.");
	}

	void PostProcess::checkFramebufferStatus(RenderCommandList& cmdList, GLuint fbo) {
		GLenum completeness;
		cmdList.checkNamedFramebufferStatus(fbo, GL_DRAW_FRAMEBUFFER, &completeness);
		// #todo-cmd-list: Don't flush here
		cmdList.flushAllCommands();
		if (completeness != GL_FRAMEBUFFER_COMPLETE) {
			LOG(LogFatal, "%s: Failed to initialize FBO for post process", __FUNCTION__);
			CHECKF(0, "Failed to initialize FBO for post process");
		}
	}

}
