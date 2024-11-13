#include "physics_component.h"
#include "actor.h"
#include "world.h"

#include "badger/physics/physics_scene.h"

namespace pathos {

	PhysicsComponent::PhysicsComponent() {
		//
	}

	PhysicsComponent::~PhysicsComponent() {
		//
	}

	void PhysicsComponent::onRegister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		body = physicsScene.allocateBody();
	}

	void PhysicsComponent::onUnregister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		physicsScene.releaseBody(body);
	}

}
