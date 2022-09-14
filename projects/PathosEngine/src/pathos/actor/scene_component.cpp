#include "scene_component.h"
#include "pathos/util/log.h"

namespace pathos {

	void SceneComponent::setLocation(const vector3& inLocation) {
		transform.setLocation(inLocation);
	}

	void SceneComponent::addLocation(const vector3& inDeltaLocation) {
		transform.setLocation(transform.getLocation() + inDeltaLocation);
	}

	void SceneComponent::setRotation(const Rotator& inRotation) {
		transform.setRotation(inRotation);
	}

	void SceneComponent::setScale(float inUniformScale) {
		transform.setScale(inUniformScale);
	}

	void SceneComponent::setScale(const vector3& inScale) {
		transform.setScale(inScale);
	}

	void SceneComponent::setTransformParent(SceneComponent* parent) {
		CHECK(parent != nullptr);
		if (getOwner() == nullptr || getOwner() != parent->getOwner()) {
			// Owner actors are different.
			LOG(LogError, "%s: Owner actors are different", __FUNCTION__);
			return;
		}
		{
			SceneComponent* ancestor = parent;
			while (ancestor != nullptr) {
				if (ancestor == this) {
					LOG(LogError, "%s: Parent wannabe is myself or my child", __FUNCTION__);
					return;
				}
				ancestor = parent->getTransformParent();
			}
		}
		// OK to register as parent.
		transformParent = parent;
		parent->transformChildren.push_back(this);
	}

	void SceneComponent::unsetTransformParent() {
		if (transformParent != nullptr) {
			auto& v = transformParent->transformChildren;
			auto it = std::find(v.begin(), v.end(), this);
			CHECK(it != v.end());
			v.erase(it);

			transformParent = nullptr;
		}
	}

	void SceneComponent::updateTransformHierarchy() {
		if (transformParent == nullptr) {
			accumulatedTransform = transform.getMatrix();
			for (SceneComponent* child : transformChildren) {
				child->accumulateTransform(accumulatedTransform);
			}
		}
	}

	void SceneComponent::accumulateTransform(const matrix4& parentTransform) {
		accumulatedTransform = parentTransform * transform.getMatrix();
		for (SceneComponent* child : transformChildren) {
			child->accumulateTransform(accumulatedTransform);
		}
	}

}
