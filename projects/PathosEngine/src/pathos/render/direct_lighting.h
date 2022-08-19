#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/shader/uniform_buffer.h"

// #todo-ssr: This pass perform lighting calculation from gbuffers.
// 1. Rename the class.
// 2. Separate local illumination and global illumination passes.
// 3. Implement screen space reflection.

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

		void render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		void createResource(RenderCommandList& cmdList);

	private:
		GLuint fbo;
		UniformBuffer ubo_directLighting;

		PlaneGeometry* quad = nullptr;

		bool destroyed = false;

	};

}
