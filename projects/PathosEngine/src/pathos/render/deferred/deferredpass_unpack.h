#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class SceneProxy;
	class Camera;

	class MeshDeferredRenderPass_Unpack {

	public:
		MeshDeferredRenderPass_Unpack();
		~MeshDeferredRenderPass_Unpack();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		// Call before render()
		void bindFramebuffer(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		void createResource(RenderCommandList& cmdList);

	private:
		GLuint fbo;
		UniformBuffer ubo_unpack;

		PlaneGeometry* quad = nullptr;

		bool destroyed = false;

	};

}
