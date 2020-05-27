#pragma once

#include "badger/math/rotator.h"

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

		inline void setVisibility(bool isVisible) { visible = isVisible; }
		inline bool getVisibility() const { return visible; }

		// Location
		void setLocation(const vector3& inLocation);
		void addLocation(const vector3& inDeltaLocation);
		inline vector3 getLocation() const { return transform.getLocation(); }

		// Rotation
		void setRotation(const Rotator& inRotation);
		inline Rotator getRotation() const { return transform.getRotation(); }

		// Scale
		void setScale(float inUniformScale);
		void setScale(const vector3& inScale);
		inline vector3 getScale() const { return transform.getScale(); }

	protected:
		matrix4 getMatrix() const;

	private:
		ModelTransform transform;
		bool visible = true;

	};

}
