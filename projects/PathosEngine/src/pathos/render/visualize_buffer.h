#pragma once

#include "pathos/core_minimal.h"
#include "pathos/rhi/uniform_buffer.h"

namespace pathos {

	class SceneProxy;
	class Camera;

	class VisualizeBufferPass {
		
	public:
		VisualizeBufferPass();
		~VisualizeBufferPass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);
		void render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		GLuint dummyVAO;
		UniformBuffer ubo;

	};

}
