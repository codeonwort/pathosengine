#include "actor.h"
#include "actor_component.h"
#include "scene_component.h"
#include "world.h"
#include "pathos/scene/scene.h"

namespace pathos {

	Actor::~Actor() {
		CHECK(markedForDeath);
	}

	void Actor::destroy() {
		destroyComponents();
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

	Rotator Actor::getActorRotation() const
	{
		return rootComponent->getRotation();
	}

	vector3 Actor::getActorScale() const {
		return rootComponent->getScale();
	}

	void Actor::setActorLocation(const vector3& inLocation) {
		rootComponent->setLocation(inLocation);
	}

	void Actor::setActorLocation(float inX, float inY, float inZ) {
		setActorLocation(vector3(inX, inY, inZ));
	}

	void Actor::setActorRotation(const Rotator& inRotation) {
		rootComponent->setRotation(inRotation);
	}

	void Actor::setActorScale(const vector3& inScale) {
		rootComponent->setScale(inScale);
	}

	void Actor::setActorScale(float inScale) {
		rootComponent->setScale(inScale);
	}

	void Actor::updateTransformHierarchy() {
		for (ActorComponent* component : components) {
			if (component->isSceneComponent()) {
				SceneComponent* sceneComponent = dynamic_cast<SceneComponent*>(component);
				sceneComponent->updateTransformHierarchy();
			}
		}
	}

	void Actor::fixRootComponent() {
		// Every actor must have a root component. If not, create one.
		if (rootComponent == nullptr) {
			bool hasSceneComponent = false;
			for (ActorComponent* component : components) {
				if (component->isSceneComponent()) {
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

	void Actor::tickComponentsPrePhysics(float deltaSeconds) {
		for (ActorComponent* comp : components) {
			if (ENUM_HAS_FLAG(comp->tickPhases, ActorComponent::ETickPhase::PrePhysics)) {
				comp->onPrePhysicsTick(deltaSeconds);
			}
		}
	}

	void Actor::tickComponentsPostPhysics(float deltaSeconds) {
		for (ActorComponent* comp : components) {
			if (ENUM_HAS_FLAG(comp->tickPhases, ActorComponent::ETickPhase::PostPhysics)) {
				comp->onPostPhysicsTick(deltaSeconds);
			}
		}
	}

	void Actor::tickComponentsPreActorTick(float deltaSeconds) {
		for (ActorComponent* comp : components) {
			if (ENUM_HAS_FLAG(comp->tickPhases, ActorComponent::ETickPhase::PreActorTick)) {
				comp->onPreActorTick(deltaSeconds);
			}
		}
	}

	void Actor::tickComponentsPostActorTick(float deltaSeconds) {
		for (ActorComponent* comp : components) {
			if (ENUM_HAS_FLAG(comp->tickPhases, ActorComponent::ETickPhase::PostActorTick)) {
				comp->onPostActorTick(deltaSeconds);
			}
		}
	}

}
