#include "post_process.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	PostProcess::~PostProcess() {
		CHECKF(resourcesDestroyed, "Child classes should override releaseResources() and set this true at the end.");
	}

	//void PostProcess::setInput(EPostProcessInput inBinding, GLuint texture)
	//{
	//	uint8 binding = (uint8)inBinding;
	//
	//	CHECK(binding < 16 && texture != 0);
	//	if(inputArray.size() <= binding) {
	//		inputArray.resize(binding + 1);
	//	}
	//	inputArray[binding] = texture;
	//}

	//void PostProcess::setOutput(EPostProcessOutput inBinding, GLuint texture)
	//{
	//	uint8 binding = (uint8)inBinding;
	//
	//	// #todo-postprocess: What if texture == 0 (backbuffer)
	//	//CHECK(binding < 8 && texture != 0);
	//	CHECK(binding < 8);
	//	if(outputArray.size() <= binding) {
	//		outputArray.resize(binding + 1);
	//	}
	//	outputArray[binding] = texture;
	//}

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
