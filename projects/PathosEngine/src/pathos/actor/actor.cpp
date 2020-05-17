#include "actor.h"
#include "actor_component.h"
#include "pathos/scene/scene.h"

namespace pathos {

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

	// #todo-actor: Optimize for the case of frequent unregistering from heavy actors
	void Actor::unregisterComponent(ActorComponent* component) {
		auto it = std::find(components.begin(), components.end(), component);
		if (it != components.end()) {
			component->onUnregister();
			component->owner = nullptr;
			components.erase(it);
		}
	}

	// #todo-actor: Implement later... when really needed
	//void Actor::unregisterComponents(std::vector<ActorComponent*>& componentsToUnregister) {
	//	std::vector<int32> indices;
	//	indices.reserve(componentsToUnregister.size());
	//
	//	for (int32 i = 0; i < components.size(); ++i) {
	//		auto it = std::find(componentsToUnregister.begin(), componentsToUnregister.end(), components[i]);
	//		if (it != componentsToUnregister.end()) {
	//			indices.push_back(static_cast<int32>(it - componentsToUnregister.begin()));
	//		}
	//	}
	//
	//	std::vector<ActorComponent*> newComponents;
	//	newComponents.reserve(components.size() - indices.size());
	//	int32 offset = 0;
	//	for (int32 i = 0; i < indices.size(); ++i) {
	//		auto it = components.begin() + indices[i] + offset;
	//		(*it)->onUnregister();
	//		(*it)->owner = nullptr;
	//		components.erase(it);
	//		--offset;
	//	}
	//}

}
