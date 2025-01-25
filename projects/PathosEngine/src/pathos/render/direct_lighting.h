#pragma once

#include "pathos/rhi/uniform_buffer.h"

// Calculate direct lighting (= local illumination) and write to sceneColor.

namespace pathos {

	class SceneProxy;
	class Camera;

	class DirectLightingPass {

	public:
		DirectLightingPass();
		~DirectLightingPass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		// Call before render()
		void bindFramebuffer(RenderCommandList& cmdList);

		void renderDirectLighting(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		void renderDirectionalLights(RenderCommandList& cmdList, SceneProxy* scene);
		void renderLocalLights(RenderCommandList& cmdList, SceneProxy* scene);

		GLuint fbo = 0xffffffff;
		UniformBuffer uboDirLight;
		UniformBuffer uboPointLight;
		UniformBuffer uboRectLight;

		bool bDestroyed = false;

	};

}
