#pragma once

#include "badger/physics/shape.h"

#include "actor_component.h"

namespace pathos {

	class PhysicsComponent : public ActorComponent {
		using EShapeType = badger::physics::Shape::EShapeType;

	public:
		PhysicsComponent();
		~PhysicsComponent();

		void setMass(float mass);
		void setElasticity(float elasticity);
		void setInfiniteMass();

		void setShapeSphere(float radius);

	protected:
		virtual void onRegister() override;
		virtual void onUnregister() override;

		virtual void onPrePhysicsTick(float deltaSeconds) override;
		virtual void onPostPhysicsTick(float deltaSeconds) override;

	private:
		void updateShape();

		badger::physics::Body* body = nullptr;
		badger::physics::Shape* shape = nullptr;

		// Maintain physics properties here and sync with badger Body.
		float invMass = 0.0f;
		float elasticity = 1.0f;

		EShapeType shapeType = EShapeType::Sphere;
		float shapeSphereRadius = 1.0f;

	};

}
