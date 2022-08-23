#pragma once

#include "pathos/render/render_command_list.h"
#include "pathos/shader/uniform_buffer.h"

// Based on GPU Pro 5, "HiZ Screen-Space Cone-Traced Reflections", Yasin Uludag.

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
		GLuint fbo_raytracing = 0xffffffff;

		GLuint pointSampler = 0;

		UniformBuffer uboHiZ;
		UniformBuffer uboRayTracing;

		bool destroyed = false;

	};

}
