#include "scene_component.h"

namespace pathos {

	void SceneComponent::setLocation(const glm::vec3& inLocation) {
		transform.setLocation(inLocation);
	}

	void SceneComponent::addLocation(const glm::vec3& inDeltaLocation) {
		transform.setLocation(transform.getLocation() + inDeltaLocation);
	}

	void SceneComponent::setScale(float inUniformScale) {
		transform.setScale(inUniformScale);
	}

	void SceneComponent::setScale(const glm::vec3& inScale) {
		transform.setScale(inScale);
	}

}
