#pragma once

#include "actor_component.h"
#include "pathos/scene/scene.h"
#include "pathos/util/engine_util.h"
#include "pathos/mesh/model_transform.h"

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

		// Location
		void setLocation(const glm::vec3& inLocation);
		void addLocation(const glm::vec3& inDeltaLocation);
		inline glm::vec3 getLocation() const { return transform.getLocation(); }

		// Rotation // #todo-actor
		//void setRotation();
		//inline Rotator getRotation() const { return (0); }

		// Scale
		void setScale(float inUniformScale);
		void setScale(const glm::vec3& inScale);
		inline glm::vec3 getScale() const { return transform.getScale(); }

	private:
		ModelTransform transform;

	};

}
