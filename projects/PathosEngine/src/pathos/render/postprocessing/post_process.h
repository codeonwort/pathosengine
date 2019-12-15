#pragma once

#include "pathos/render/render_command_list.h"
#include "pathos/mesh/geometry_primitive.h"

#include "badger/types/int_types.h"
#include <vector>

namespace pathos {

	enum class EPostProcessInput : uint8 {
		PPI_0 = 0,
		PPI_1 = 1,
		PPI_2 = 2,
		PPI_3 = 3
	};

	enum class EPostProcessOutput : uint8 {
		PPO_0 = 0,
		PPO_1 = 1,
		PPO_2 = 2,
		PPO_3 = 3
	};

	class PostProcess {
		
	public:
		virtual ~PostProcess();

		//void setInput(EPostProcessInput binding, GLuint texture);
		//void setOutput(EPostProcessOutput binding, GLuint texture);

		virtual void initializeResources(RenderCommandList& cmdList) = 0;
		virtual void releaseResources(RenderCommandList& cmdList) = 0;
		virtual void renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) = 0;

	protected:
		void checkFramebufferStatus(RenderCommandList& cmdList, GLuint fbo);

		__forceinline void markDestroyed()
		{
			resourcesDestroyed = true;
		}

		//__forceinline GLuint getInput(EPostProcessInput binding) const
		//{
		//	return inputArray[(uint8)binding];
		//}
		//__forceinline GLuint getOutput(EPostProcessOutput binding) const
		//{
		//	return outputArray[(uint8)binding];
		//}

	private:
		// #todo-postprocess: Child classes should override releaseResources() and set this true at the end
		bool resourcesDestroyed = false;

		std::vector<GLuint> inputArray;
		std::vector<GLuint> outputArray;

	};

}
