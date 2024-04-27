#pragma once

#include "post_process.h"
#include "pathos/rhi/uniform_buffer.h"

#include "badger/types/vector_types.h"
#include "badger/types/noncopyable.h"

namespace pathos {

	// Screen Space Ambient Occlusion pass.
	class SSAO final : public Noncopyable {

	public:
		static constexpr uint32 MAX_SAMPLE_POINTS = 64;
		static constexpr uint32 NUM_ROTATION_NOISE = 16;

		struct UBO_SSAO_Random {
			static constexpr GLuint BINDING_INDEX = 2;

			// For Vogel disk. w component is not used.
			vector4 randomRotations[NUM_ROTATION_NOISE];
		};

	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);
		void renderAmbientOcclusion(RenderCommandList& cmdList);

	private:
		GLuint fboBlur = 0;
		GLuint fboBlur2 = 0;

		UniformBuffer ubo;
		UniformBuffer uboRandom;

		UBO_SSAO_Random randomData;
		bool bRandomDataValid = false;

	};

}
