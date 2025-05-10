#include "physics_component.h"
#include "actor.h"
#include "world.h"
#include "static_mesh_component.h"
#include "pathos/mesh/static_mesh.h"
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
		bRecreateShape = true;
	}

	void PhysicsComponent::setShapeBox(const vector3& extents) {
		shapeType = EShapeType::Box;
		shapeBoxExtents = extents;
		bRecreateShape = true;
	}

	void PhysicsComponent::onRegister() {
		auto& physicsScene = getOwner()->getWorld()->getPhysicsScene();
		body = physicsScene.allocateBody();

		MeshGeometry* G_bounds = gEngine->getSystemGeometryUnitCube();
		auto M_bounds = Material::createMaterialInstance("unlit");
		M_bounds->setConstantParameter("color", vector3(0.1f, 1.0f, 0.1f));
		M_bounds->bWireframe = true;

		boundsComponent = new StaticMeshComponent;
		boundsComponent->setStaticMesh(makeAssetPtr<StaticMesh>(G_bounds, M_bounds));
		boundsComponent->setVisibility(false);
		boundsComponent->castsShadow = false;
		getOwner()->registerComponent(boundsComponent);
		boundsComponent->setTransformParent(getOwner()->getRootComponent());
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
		body->setElasticity(elasticity);
		body->setFriction(friction);
		if (bForceLinearVelocity) {
			bForceLinearVelocity = false;
			body->setLinearVelocity(forcedLinearVelocity);
		}

		if (shapeType == EShapeType::Sphere) {
			boundsComponent->setVisibility(cvarShowPhysicsBodyBounds.getInt() != 0);
			boundsComponent->setScale(shapeSphereRadius);
		} else if (shapeType == EShapeType::Box) {
			boundsComponent->setVisibility(cvarShowPhysicsBodyBounds.getInt() != 0);
			boundsComponent->setScale(0.5f * shapeBoxExtents);
		}
	}

	void PhysicsComponent::onPostPhysicsTick(float deltaSeconds) {
		glm::quat orientation = body->getOrientation();
		vector3 dir(orientation.x, orientation.y, orientation.z);
		Rotator rot = Rotator::directionToYawPitch(dir);

		getOwner()->setActorLocation(body->getCenterOfMassWorldSpace());
		getOwner()->setActorRotation(rot);
	}

	void PhysicsComponent::updateShape() {
		if (bRecreateShape) {
			bRecreateShape = false;
			if (shape != nullptr) {
				delete shape;
				shape = nullptr;
			}
		}
		if (shape == nullptr) {
			if (shapeType == EShapeType::Sphere) {
				shape = new badger::physics::ShapeSphere(shapeSphereRadius);
			} else if (shapeType == EShapeType::Box) {
				shape = new badger::physics::ShapeBox(shapeBoxExtents);
			} else {
				CHECK_NO_ENTRY();
			}
			body->setShape(shape);
		}
	}

}
