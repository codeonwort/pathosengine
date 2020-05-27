#include "scene_component.h"

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

	matrix4 SceneComponent::getMatrix() const {
		return transform.getMatrix();
	}

}
