#pragma once

#include "badger/physics/shape.h"

#include "actor_component.h"

namespace pathos {

	class StaticMesh;
	class StaticMeshComponent;

	class PhysicsComponent : public ActorComponent {
		using EShapeType = badger::physics::Shape::EShapeType;

	public:
		PhysicsComponent();
		~PhysicsComponent();

		void setMass(float mass);
		void setInfiniteMass();
		void setElasticity(float elasticity);
		void setFriction(float friction);

		void forceLinearVelocity(const vector3& value);

		void setShapeSphere(float radius);
		void setShapeBox(const vector3& extents);

	protected:
		virtual void onRegister() override;
		virtual void onUnregister() override;

		virtual void onPrePhysicsTick(float deltaSeconds) override;
		virtual void onPostPhysicsTick(float deltaSeconds) override;

	private:
		void updateShape();

		badger::physics::Body* body = nullptr;
		badger::physics::Shape* shape = nullptr;

		StaticMeshComponent* boundsComponent = nullptr;
		StaticMesh* boundsMesh = nullptr;

		// Maintain physics properties here and sync with badger Body.
		float invMass = 0.0f;
		float elasticity = 1.0f;
		float friction = 1.0f;

		vector3 forcedLinearVelocity = vector3(0.0f);
		bool bForceLinearVelocity = false;

		EShapeType shapeType = EShapeType::Sphere;
		bool bRecreateShape = false;
		float shapeSphereRadius = 1.0f;
		vector3 shapeBoxExtents = vector3(1.0f);

	};

}
