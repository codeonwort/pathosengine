#pragma once

#include "post_process.h"
#include "pathos/rhi/uniform_buffer.h"


namespace pathos {

	class DepthOfField : public PostProcess {

	public:
		~DepthOfField();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

		bool isAvailable() const;

	private:
		GLuint createBlurShader();

		GLuint vao = 0;
		GLuint fbo = 0;
		UniformBuffer uboPrefixSum;
		UniformBuffer uboBlur;

	};

}
