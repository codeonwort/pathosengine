#pragma once

#include "post_process.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class SSAO : public PostProcess {

		struct UBO_SSAO_Random {
			glm::vec4 points[256];
			glm::vec4 randomVectors[256];
		};

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint program_downscale = 0xffffffff;
		GLuint program_ao = 0xffffffff;
		GLuint program_blur = 0xffffffff;
		GLuint program_blur2 = 0xffffffff;
		GLuint fboBlur = 0;
		GLuint fboBlur2 = 0;
		UniformBuffer ubo;
		UniformBuffer uboRandom;

		UBO_SSAO_Random randomData;
		bool randomGenerated = false;

	};

}
