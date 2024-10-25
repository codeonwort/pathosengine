#pragma once

#include "pathos/rhi/uniform_buffer.h"

namespace pathos {

	class SceneProxy;
	class Camera;
	class MeshGeometry;
	struct SceneRenderTargets;

	// Used by SceneRenderer for various landscape processing.
	// Actual rendering logic might be spread across render passes. (e.g., GBufferPass)
	class LandscapeRendering final {

	public:
		LandscapeRendering();
		~LandscapeRendering();

		void initializeResources(RenderCommandList& cmdList);

		void releaseResources(RenderCommandList& cmdList);

		// Call at the start of SceneRenderer::renderScene() so that
		// it precedes any mesh rendering (e.g., depth prepass, gbuffer pass, ...) passes.
		void preprocess(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		UniformBuffer uboLandscapeCulling;

	};

}
