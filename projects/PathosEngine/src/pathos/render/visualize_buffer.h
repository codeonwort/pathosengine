#pragma once

#include "pathos/core_minimal.h"

namespace pathos {

	class SceneProxy;
	class Camera;

	class VisualizeBufferPass {
		
	public:
		VisualizeBufferPass();
		~VisualizeBufferPass();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);
		void render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		GLuint dummyVAO;

	};

}
