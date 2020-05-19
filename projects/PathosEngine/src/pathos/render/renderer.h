#pragma once

#include "pathos/render/render_command_list.h"
#include "pathos/material/material_id.h"

#include <vector>

namespace pathos {

	class Scene;
	class Camera;

	class Renderer {
		
	public:
		Renderer()                = default;
		virtual ~Renderer()       = default;

		Renderer(const Renderer&)            = delete;
		Renderer& operator=(const Renderer&) = delete;

		virtual void initializeResources(RenderCommandList& cmdList) = 0;
		virtual void releaseResources(RenderCommandList& cmdList) = 0;
		virtual void render(RenderCommandList& cmdList, Scene* scene, Camera* camera) = 0;

	};

}
