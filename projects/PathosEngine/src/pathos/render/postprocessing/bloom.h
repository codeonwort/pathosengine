#pragma once

#include "post_process.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class BloomPass : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint fbo;
		UniformBuffer uboUpsample;

	};

}
