#pragma once

#include "actor_component.h"
#include "pathos/scene/scene.h"
#include "pathos/util/engine_util.h"
#include "pathos/mesh/model_transform.h"

#include "badger/math/rotator.h"
#include <vector>

namespace pathos {

	// SceneComponent metadata for render thread
	struct SceneComponentProxy {
		//
	};

	// Component that can be viewed in a scene
	class SceneComponent : public ActorComponent {
		
	public:
		SceneComponent() = default;
		virtual ~SceneComponent() = default;

		bool isSceneComponent() const override { return true; }

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

		// NOTE: Only components in the same actor are valid as children.
		//       Also this relationship is only for transform hierarchy.
		void setTransformParent(SceneComponent* parent);
		void unsetTransformParent();
		inline SceneComponent* getTransformParent() const { return transformParent; }
		void updateTransformHierarchy();

	protected:
		inline matrix4 getLocalMatrix() const { return accumulatedTransform; }
		//inline matrix4 getLocalMatrix() const { return transform.getMatrix(); }
		//inline matrix4 getAccumulatedLocalMatrix() const { return accumulatedTransform; }

	private:
		void accumulateTransform(const matrix4& parentTransform);

	private:
		ModelTransform transform;
		bool visible = true;

		// For transform hierarchy.
		matrix4 accumulatedTransform;
		// #todo: Should I always accumulate root's transform
		//        even if root is not the transform parent?
		SceneComponent* transformParent = nullptr;
		std::vector<SceneComponent*> transformChildren;
	};

}
