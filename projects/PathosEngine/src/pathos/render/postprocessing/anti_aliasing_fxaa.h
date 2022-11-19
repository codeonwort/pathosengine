#pragma once

#include "post_process.h"
#include "pathos/rhi/uniform_buffer.h"

// NVidia FXAA wrapper

namespace pathos {

	class FXAA final : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint fbo = 0;
		UniformBuffer ubo;

	};

}
