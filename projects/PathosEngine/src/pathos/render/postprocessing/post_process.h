#pragma once

#include "pathos/render/render_command_list.h"
#include "pathos/mesh/geometry_primitive.h"

#include "badger/types/int_types.h"
#include <vector>

namespace pathos {

	enum class EPostProcessInput : uint8 {
		PPI_0 = 0,
		PPI_1,
		PPI_2,
		PPI_3,
		PPI_4,
		PPI_5,
		PPI_6,
		PPI_7,
		PPI_8,
		PPI_9,
		PPI_10,
		PPI_11,
		PPI_12,
		PPI_13,
		PPI_14,
		PPI_15,
		PPI_16,
		PPI_17,
		PPI_18,
		PPI_19,
		PPI_20,
		PPI_21,
		PPI_22,
		PPI_23,
		PPI_24,
		PPI_25,
		PPI_26,
		PPI_27,
		PPI_28,
		PPI_29,
		PPI_30,
		PPI_31,
		PPI_MAX = PPI_31
	};
	static_assert(static_cast<uint32>(EPostProcessInput::PPI_MAX) <= 255, "too many PPIs");

	enum class EPostProcessOutput : uint8 {
		PPO_0 = 0,
		PPO_1,
		PPO_2,
		PPO_3,
		PPO_4,
		PPO_5,
		PPO_6,
		PPO_7,
		PPO_8,
		PPO_9,
		PPO_10,
		PPO_11,
		PPO_12,
		PPO_13,
		PPO_14,
		PPO_15,
		PPO_16,
		PPO_17,
		PPO_18,
		PPO_19,
		PPO_20,
		PPO_21,
		PPO_22,
		PPO_23,
		PPO_24,
		PPO_25,
		PPO_26,
		PPO_27,
		PPO_28,
		PPO_29,
		PPO_30,
		PPO_31,
		PPO_MAX = PPO_31
	};
	static_assert(static_cast<uint32>(EPostProcessOutput::PPO_MAX) <= 255, "too many PPOs");

	// I'll not distinguish post processing and screen-space techniques here
	// All of them are subclasses of PostProcess
	class PostProcess {
		
	public:
		virtual ~PostProcess();

		virtual void initializeResources(RenderCommandList& cmdList) = 0;
		virtual void releaseResources(RenderCommandList& cmdList) = 0;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) = 0;

		__forceinline void setInput(EPostProcessInput binding, GLuint textureSRV) {
			CHECKF((uint8)binding <= (uint8)EPostProcessInput::PPI_MAX, "binding is invalid");

			inputArray[(uint32)binding] = textureSRV;
		}

		__forceinline void setOutput(EPostProcessOutput binding, GLuint textureSRV) {
			CHECKF((uint8)binding <= (uint8)EPostProcessOutput::PPO_MAX, "binding is invalid");

			outputArray[(uint32)binding] = textureSRV;
		}

	protected:
		__forceinline void markDestroyed()
		{
			resourcesDestroyed = true;
		}

		__forceinline GLuint getInput(EPostProcessInput binding) const
		{
			return inputArray[(uint32)binding];
		}
		__forceinline GLuint getOutput(EPostProcessOutput binding) const
		{
			return outputArray[(uint32)binding];
		}

	private:
		// #todo-postprocess: Child classes should override releaseResources() and set this true at the end
		bool resourcesDestroyed = false;

		GLuint inputArray[32];
		GLuint outputArray[32];

	};

}
