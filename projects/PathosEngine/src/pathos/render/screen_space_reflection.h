#pragma once

#include "pathos/render/render_command_list.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class Camera;
	class SceneProxy;
	class MeshGeometry;

	class ScreenSpaceReflectionPass {
		
	public:
		ScreenSpaceReflectionPass();
		~ScreenSpaceReflectionPass();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void renderScreenSpaceReflection(
			RenderCommandList& cmdList,
			SceneProxy* scene,
			Camera* camera,
			MeshGeometry* fullscreenQuad);

	private:
		GLuint fbo_HiZ = 0xffffffff;
		GLuint fbo_preintegration = 0xffffffff;

		bool destroyed = false;

	};

}
