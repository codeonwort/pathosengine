#include "physics_component.h"
#include "actor.h"
#include "world.h"
#include "static_mesh_component.h"
#include "pathos/mesh/mesh.h"
#include "pathos/material/material.h"
#include "pathos/console.h"

#include "badger/physics/physics_scene.h"
#include "badger/physics/shape.h"

namespace pathos {

	static ConsoleVariable<int32> cvarShowPhysicsBodyBounds("physics.showBounds", 1, "Show bounds of physical bodies");

	PhysicsComponent::PhysicsComponent() {
		setTickPhases(ActorComponent::ETickPhase::PrePhysics | ActorComponent::ETickPhase::PostPhysics);
	}

	PhysicsComponent::~PhysicsComponent() {
		//
	}

	void PhysicsComponent::setMass(float mass) { invMass = 1.0f / mass; }
	void PhysicsComponent::setInfiniteMass() { invMass = 0.0f; }
	void PhysicsComponent::setElasticity(float elasticity) { this->elasticity = elasticity; }
	void PhysicsComponent::setFriction(float friction) { this->friction = friction; }

	void PhysicsComponent::forceLinearVelocity(const vector3& value) {
		forcedLinearVelocity = value;
		bForceLinearVelocity = true;
	}

	void PhysicsComponent::setShapeSphere(float radius) {
		shapeType = EShapeType::Sphere;
		shapeSphereRadius = radius;
	}

	void PhysicsComponent::onRegister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		body = physicsScene.allocateBody();

		MeshGeometry* G_bounds = gEngine->getSystemGeometryUnitCube();
		Material* M_bounds = Material::createMaterialInstance("unlit");
		M_bounds->setConstantParameter("color", vector3(0.1f, 1.0f, 0.1f));
		M_bounds->bWireframe = true;

		boundsComponent = new StaticMeshComponent;
		boundsComponent->setStaticMesh(new Mesh(G_bounds, M_bounds));
		boundsComponent->setVisibility(false);
		boundsComponent->castsShadow = false;
		getOwner()->registerComponent(boundsComponent);
		boundsComponent->setTransformParent(getOwner()->getRootComponent());
	}

	void PhysicsComponent::onUnregister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		physicsScene.releaseBody(body);
		if (shape != nullptr) delete shape;
		if (boundsComponent != nullptr) {
			getOwner()->unregisterComponent(boundsComponent);
			delete boundsComponent;
		}
	}

	void PhysicsComponent::onPrePhysicsTick(float deltaSeconds) {
		updateShape();
		body->setPosition(getOwner()->getActorLocation());
		body->setInvMass(invMass);
		body->setElasticity(elasticity);
		body->setFriction(friction);
		if (bForceLinearVelocity) {
			bForceLinearVelocity = false;
			body->setLinearVelocity(forcedLinearVelocity);
		}

		if (shapeType == EShapeType::Sphere) {
			boundsComponent->setVisibility(cvarShowPhysicsBodyBounds.getInt() != 0);
			boundsComponent->setScale(shapeSphereRadius);
		}
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
