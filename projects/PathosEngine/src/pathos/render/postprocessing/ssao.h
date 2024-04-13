#pragma once

#include "post_process.h"
#include "pathos/rhi/uniform_buffer.h"

#include "badger/types/vector_types.h"

namespace pathos {

	// Screen Space Ambient Occlusion pass.
	class SSAO : public PostProcess {

	public:
		static constexpr uint32 MAX_SAMPLE_POINTS = 64;
		static constexpr uint32 NUM_ROTATION_NOISE = 16;

		struct UBO_SSAO_Random {
			static constexpr GLuint BINDING_INDEX = 2;

			// For Vogel disk. w component is not used.
			vector4 randomRotations[NUM_ROTATION_NOISE];
		};

	public:
		virtual void initializeResources(RenderCommandList& cmdList) override;
		virtual void releaseResources(RenderCommandList& cmdList) override;
		virtual void renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) override;

	private:
		GLuint fboBlur = 0;
		GLuint fboBlur2 = 0;

		UniformBuffer ubo;
		UniformBuffer uboRandom;

		UBO_SSAO_Random randomData;
		bool bRandomDataValid = false;

	};

}
