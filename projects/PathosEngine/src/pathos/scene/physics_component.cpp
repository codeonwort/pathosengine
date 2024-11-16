#include "physics_component.h"
#include "actor.h"
#include "world.h"

#include "badger/physics/physics_scene.h"
#include "badger/physics/shape.h"

namespace pathos {

	PhysicsComponent::PhysicsComponent() {
		setTickPhases(ActorComponent::ETickPhase::PostPhysics);
	}

	PhysicsComponent::~PhysicsComponent() {
		//
	}

	void PhysicsComponent::onRegister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		body = physicsScene.allocateBody();
		// #todo-physics: Temp sphere shape
		body->setShape(new badger::physics::ShapeSphere(1.0f));
	}

	void PhysicsComponent::onUnregister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		physicsScene.releaseBody(body);
	}

	void PhysicsComponent::onPostPhysicsTick(float deltaSeconds) {
		getOwner()->setActorLocation(body->getCenterOfMassWorldSpace());
	}

}
