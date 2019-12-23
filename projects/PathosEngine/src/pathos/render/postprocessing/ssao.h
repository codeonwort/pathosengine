#pragma once

#include "post_process.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class SSAO : public PostProcess {

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint program_downscale = 0xffffffff;
		GLuint program_ao = 0xffffffff;
		GLuint fbo = 0;
		UniformBuffer ubo;

	};

}
