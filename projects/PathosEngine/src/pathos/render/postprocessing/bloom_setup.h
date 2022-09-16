#pragma once

#include "post_process.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class BloomSetup : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

		void clearSceneBloom(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad);

	private:
		GLuint fbo = 0;
		UniformBuffer ubo;

	};
	
}
