#include "physics_component.h"
#include "actor.h"
#include "world.h"

#include "badger/physics/physics_scene.h"
#include "badger/physics/shape.h"

namespace pathos {

	PhysicsComponent::PhysicsComponent() {
		setTickPhases(ActorComponent::ETickPhase::PrePhysics | ActorComponent::ETickPhase::PostPhysics);
	}

	PhysicsComponent::~PhysicsComponent() {
		//
	}

	void PhysicsComponent::setMass(float mass) { invMass = 1.0f / mass; }
	void PhysicsComponent::setInfiniteMass() { invMass = 0.0f; }

	void PhysicsComponent::setShapeSphere(float radius) {
		shapeType = EShapeType::Sphere;
		shapeSphereRadius = radius;
	}

	void PhysicsComponent::onRegister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		body = physicsScene.allocateBody();
	}

	void PhysicsComponent::onUnregister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		physicsScene.releaseBody(body);
		if (shape != nullptr) delete shape;
	}

	void PhysicsComponent::onPrePhysicsTick(float deltaSeconds) {
		updateShape();
		body->setPosition(getOwner()->getActorLocation());
		body->setInvMass(invMass);
	}

	void PhysicsComponent::onPostPhysicsTick(float deltaSeconds) {
		getOwner()->setActorLocation(body->getCenterOfMassWorldSpace());
	}

	void PhysicsComponent::updateShape() {
		if (shape == nullptr) {
			if (shapeType == EShapeType::Sphere) {
				shape = new badger::physics::ShapeSphere(shapeSphereRadius);
			} else {
				CHECK_NO_ENTRY();
			}
			body->setShape(shape);
		} else {
			if (shapeType == EShapeType::Sphere) {
				((badger::physics::ShapeSphere*)shape)->setRadius(shapeSphereRadius);
			}
		}
	}

}
