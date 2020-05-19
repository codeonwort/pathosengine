#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/scene/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class MeshDeferredRenderPass_Unpack {

	public:
		MeshDeferredRenderPass_Unpack();
		~MeshDeferredRenderPass_Unpack();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		// Call before render()
		void bindFramebuffer(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, Scene* scene, Camera* camera);

	private:
		void createProgram();
		void createResource(RenderCommandList& cmdList);

	private:

		GLuint program = 0;
		GLuint fbo;
		UniformBuffer ubo_unpack;

		PlaneGeometry* quad = nullptr;

		bool destroyed = false;

	};

}
