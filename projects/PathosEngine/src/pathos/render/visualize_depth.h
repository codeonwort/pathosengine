#pragma once

#include "scene.h"
#include "pathos/camera/camera.h"

namespace pathos {

	class VisualizeDepth {
		
	public:
		VisualizeDepth();
		~VisualizeDepth();

		void render(RenderCommandList& cmdList, Scene* scene, Camera* camera);

	private:
		GLuint program;
		GLuint ubo;

	};

}
