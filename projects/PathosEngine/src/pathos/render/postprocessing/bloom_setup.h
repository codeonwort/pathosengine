#pragma once

#include "post_process.h"
#include "pathos/rhi/uniform_buffer.h"

namespace pathos {

	class BloomSetup : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) override;

	private:
		GLuint fbo = 0;
		UniformBuffer ubo;

	};
	
}
