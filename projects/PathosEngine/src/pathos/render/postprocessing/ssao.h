#pragma once

#include "post_process.h"
#include "pathos/shader/uniform_buffer.h"

#include "badger/types/vector_types.h"

#define SSAO_MAX_SAMPLE_POINTS  64u
#define SSAO_NUM_ROTATION_NOISE 16u

namespace pathos {

	class SSAO : public PostProcess {

		struct UBO_SSAO_Random {
			// For Vogel disk. w component is not used.
			vector4 randomRotations[SSAO_NUM_ROTATION_NOISE];
		};

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) override;

	private:
		GLuint fboBlur = 0;
		GLuint fboBlur2 = 0;

		UniformBuffer ubo;
		UniformBuffer uboRandom;

		UBO_SSAO_Random randomData;
		bool bRandomDataValid = false;

	};

}
