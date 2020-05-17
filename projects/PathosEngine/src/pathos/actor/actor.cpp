#include "actor.h"
#include "actor_component.h"
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

}
