#pragma once

#include "post_process.h"

namespace pathos {

	class BloomPass : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint program = 0xffffffff;
		GLuint program2 = 0xffffffff;
		GLuint fbo = 0;

	};

}
