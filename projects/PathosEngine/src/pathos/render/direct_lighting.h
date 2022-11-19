#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"

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
		GLuint fbo;
		UniformBuffer uboDirLight;
		UniformBuffer uboPointLight;
		UniformBuffer uboRectLight;

		PlaneGeometry* quad = nullptr;

		bool destroyed = false;

	};

}
