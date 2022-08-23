#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/shader/uniform_buffer.h"

// Calculate direct lighting (= local illumination) and write to sceneColor.

namespace pathos {

	class SceneProxy;
	class Camera;

	class DirectLightingPass {

	public:
		DirectLightingPass();
		~DirectLightingPass();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		// Call before render()
		void bindFramebuffer(RenderCommandList& cmdList);

		void renderDirectLighting(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		GLuint fbo;
		UniformBuffer ubo;

		PlaneGeometry* quad = nullptr;

		bool destroyed = false;

	};

}