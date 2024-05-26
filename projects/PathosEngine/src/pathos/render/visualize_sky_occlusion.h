#pragma once

// NOTE: Which term to use between Sky Visibility and Sky Occlusion?
// I chose 'occlusion' because it aligns with other terms such as Ambient Occlusion and Specular Occlusion,
// though in formula 'visibility' is more intuitive (0 = fully occluded, 1 = fully visible).

namespace pathos {

	class SceneProxy;
	class Camera;

	// #wip-skyocclusion: Implement vis pass
	class VisualizeSkyOcclusionPass {

	public:
		VisualizeSkyOcclusionPass();
		~VisualizeSkyOcclusionPass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderSkyOcclusion(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		//

	};

}
