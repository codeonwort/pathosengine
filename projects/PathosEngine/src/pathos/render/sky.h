#pragma once

#include "pathos/scene/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/actor/actor.h"

namespace pathos {

	class SkyActor : public Actor {
		
	public:
		virtual void render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) = 0;

	};

}
