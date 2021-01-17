#pragma once

#include "pathos/scene/scene.h"
#include "pathos/camera/camera.h"

namespace pathos {

	class VisualizeDepth {
		
	public:
		VisualizeDepth();
		~VisualizeDepth();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);
		void render(RenderCommandList& cmdList, Scene* scene, Camera* camera);

	private:
		GLuint dummyVAO;

	};

}
