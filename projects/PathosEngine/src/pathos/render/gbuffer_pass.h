#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/gl_handles.h"

namespace pathos {

	class SceneProxy;
	class LandscapeRendering;
	struct SceneRenderTargets;

	class GBufferPass final {

	public:
		GBufferPass();
		~GBufferPass();

		void initializeResources(RenderCommandList& cmdList);

		void releaseResources(RenderCommandList& cmdList);

		void renderGBuffers(RenderCommandList& cmdList, SceneProxy* scene, bool hasDepthPrepass, LandscapeRendering* landscapeRendering);

	private:
		void updateFramebufferAttachments(RenderCommandList& cmdList, SceneRenderTargets* sceneRenderTargets);

	private:
		GLuint fbo = 0;
		UniformBuffer uboPerObject;

	};

}
