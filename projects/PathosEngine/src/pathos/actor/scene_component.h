#pragma once

#include "actor_component.h"
#include "pathos/scene/scene.h"
#include "pathos/util/engine_util.h"

namespace pathos {

	class Scene;

	// SceneComponent metadata for render thread
	struct SceneComponentProxy {
		//
	};

	// Component that can be viewed in a scene
	class SceneComponent : public ActorComponent {
		
	public:
		SceneComponent() = default;
		virtual ~SceneComponent() = default;

	};

}
