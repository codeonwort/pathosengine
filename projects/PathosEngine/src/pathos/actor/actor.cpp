#include "actor.h"
#include "actor_component.h"
#include "scene_component.h"
#include "pathos/scene/scene.h"

namespace pathos {

	Actor::~Actor() {
		destroyComponents();
	}

	void Actor::destroy() {
		owner->destroyActor(this);
	}

	void Actor::registerComponent(ActorComponent* component) {
		if (component->owner != nullptr) {
			component->owner->unregisterComponent(component);
		}

		components.push_back(component);
		component->owner = this;
		component->onRegister();
	}

	void Actor::unregisterComponent(ActorComponent* component) {
		auto it = std::find(components.begin(), components.end(), component);
		if (it != components.end()) {
			component->onUnregister();
			component->owner = nullptr;
			components.erase(it);
		}
	}

	void Actor::destroyComponents() {
		for (ActorComponent* component : components) {
			component->onUnregister();
			delete component;
		}
		components.clear();
	}

	void Actor::setAsRootComponent(SceneComponent* sceneComponent) {
		CHECKF(isInConstructor, "Call only in constructor");

		rootComponent = sceneComponent;
	}

	vector3 Actor::getActorLocation() const {
		return rootComponent->getLocation();
	}

	vector3 Actor::getActorScale() const {
		return rootComponent->getScale();
	}

	void Actor::setActorLocation(const vector3& inLocation) {
		rootComponent->setLocation(inLocation);
	}

	void Actor::setActorRotation(float inAngle_radians, const vector3& inAxis) {
		rootComponent->setRotation(inAngle_radians, inAxis);
	}

	void Actor::setActorScale(const vector3& inScale) {
		rootComponent->setScale(inScale);
	}

	void Actor::setActorScale(float inScale) {
		rootComponent->setScale(inScale);
	}

	void Actor::fixRootComponent() {
		// Every actor must have a root component. If not, create one.
		if (rootComponent == nullptr) {
			bool hasSceneComponent = false;
			for (ActorComponent* component : components) {
				// #todo-rtti: dynamic_cast due to no custom reflection :(
				if (dynamic_cast<SceneComponent*>(component) != nullptr) {
					hasSceneComponent = true;
					break;
				}
			}

			CHECKF(!hasSceneComponent, "You created scene component(s), but none of them are set to the root component.");

			if (!hasSceneComponent) {
				rootComponent = new SceneComponent;
				registerComponent(rootComponent);
			}
		}
	}

}
