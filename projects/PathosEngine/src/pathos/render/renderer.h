#pragma once

#include "badger/types/noncopyable.h"

#include "pathos/render/render_command_list.h"
#include "pathos/material/material_id.h"

namespace pathos {

	class Scene;
	class Camera;

	class Renderer : public Noncopyable {
		
	public:
		Renderer()                = default;
		virtual ~Renderer()       = default;

		virtual void initializeResources(RenderCommandList& cmdList) = 0;
		virtual void releaseResources(RenderCommandList& cmdList) = 0;
		virtual void render(RenderCommandList& cmdList, Scene* scene, Camera* camera) = 0;

	};

}
