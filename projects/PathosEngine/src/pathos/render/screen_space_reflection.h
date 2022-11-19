#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"

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
		void releaseResources(RenderCommandList& cmdList);

		void renderScreenSpaceReflection(
			RenderCommandList& cmdList,
			SceneProxy* scene,
			Camera* camera,
			MeshGeometry* fullscreenQuad);

	private:
		GLuint fbo_HiZ = 0xffffffff;
		GLuint fbo_preintegration = 0xffffffff;
		GLuint fbo_preconvolution = 0xffffffff;
		GLuint fbo_raytracing = 0xffffffff;
		GLuint fbo_composite = 0xffffffff;

		GLuint pointSampler = 0;
		GLuint linearSampler = 0;

		UniformBuffer uboHiZ;
		UniformBuffer uboRayTracing;

		bool destroyed = false;

	};

}
