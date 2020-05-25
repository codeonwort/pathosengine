#pragma once

#include "badger/types/noncopyable.h"

#include "pathos/scene/scene_render_settings.h"
#include "pathos/render/render_command_list.h"
#include "pathos/material/material_id.h"

namespace pathos {

	class Scene;
	class Camera;
	class RenderTarget2D;

	class Renderer : public Noncopyable {
		
	public:
		Renderer()                = default;
		virtual ~Renderer()       = default;

		virtual void initializeResources(RenderCommandList& cmdList) = 0;
		virtual void releaseResources(RenderCommandList& cmdList) = 0;

		virtual void setSceneRenderSettings(const SceneRenderSettings& settings) = 0;
		virtual void setFinalRenderTarget(RenderTarget2D* finalRenderTarget) = 0; // Default backbuffer or custom off-screen render target
		virtual void render(RenderCommandList& cmdList, Scene* scene, Camera* camera) = 0;

	};

}
