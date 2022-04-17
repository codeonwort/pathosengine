#include "engine_util.h"
#include "log.h"

namespace pathos {

	void checkFramebufferStatus(RenderCommandList& cmdList, GLuint fbo, const char* message) {
		struct FBOStatusQuery {
			GLenum* completeness;
			const char* message;
		};

		FBOStatusQuery query;
		query.completeness = ALLOC_RENDER_PROXY<GLenum>(cmdList.sceneProxy);
		query.message = ALLOC_PDO_STRING(cmdList.sceneProxy->renderProxyAllocator, message);
		cmdList.checkNamedFramebufferStatus(fbo, GL_DRAW_FRAMEBUFFER, query.completeness);

		cmdList.registerHook([query](RenderCommandList& cmdList) {
			if (*(query.completeness) != GL_FRAMEBUFFER_COMPLETE) {
				LOG(LogFatal, "Failed to initialize fbo: %s", query.message != nullptr ? query.message : "<noname>");
				CHECK_NO_ENTRY();
			}
		});
	}

}
