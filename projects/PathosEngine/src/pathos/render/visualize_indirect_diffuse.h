#pragma once

#include "pathos/rhi/uniform_buffer.h"

// NOTE: Which term to use between Sky Visibility and Sky Occlusion?
// I chose 'occlusion' because it aligns with other terms such as Ambient Occlusion and Specular Occlusion,
// though in formula 'visibility' is more intuitive (0 = fully occluded, 1 = fully visible).

namespace pathos {

	class SceneProxy;
	class Camera;

	class VisualizeIndirectDiffuse {

	public:
		VisualizeIndirectDiffuse();
		~VisualizeIndirectDiffuse();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderVisualization(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		GLuint fbo = 0;
		UniformBuffer ubo;

	};

}
