#pragma once

#include "post_process.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/uniform_buffer.h"


namespace pathos {

	class DepthOfField : public PostProcess {

	public:
		~DepthOfField();

		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint createSubsumShader();
		GLuint createBlurShader();

		GLuint vao = 0;
		GLuint fbo = 0;
		GLuint program_subsum2D = 0;
		GLuint program_blur = 0;
		UniformBuffer uboBlur;

	};

}
