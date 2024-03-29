#pragma once

#include "pathos/rhi/render_command_list.h"
#include "pathos/scene/scene_render_settings.h"
#include "pathos/material/material_id.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	struct SceneRenderTargets;
	class SceneProxy;
	class Camera;
	class RenderTargetView;

	class Renderer : public Noncopyable {
		
	public:
		Renderer()                = default;
		virtual ~Renderer()       = default;

		virtual void initializeResources(RenderCommandList& cmdList) = 0;
		virtual void releaseResources(RenderCommandList& cmdList) = 0;

		virtual void setSceneRenderSettings(const SceneRenderSettings& settings) = 0;
		virtual void setFinalRenderTarget(RenderTargetView* finalRenderTarget) = 0; // For custom off-screen render target
		virtual void setFinalRenderTargetToBackbuffer() = 0; // Final render to the default backbuffer

		virtual void renderScene(
			RenderCommandList& cmdList,
			SceneRenderTargets* sceneRenderTargets,
			SceneProxy* scene,
			Camera* camera) = 0;

	};

}
