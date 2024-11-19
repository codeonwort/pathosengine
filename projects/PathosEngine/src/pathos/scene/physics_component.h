#pragma once

#include "badger/physics/shape.h"

#include "actor_component.h"

namespace pathos {

	class PhysicsComponent : public ActorComponent {

	public:
		PhysicsComponent();
		~PhysicsComponent();

		void setMass(float mass);
		void setInfiniteMass();

	protected:
		virtual void onRegister() override;
		virtual void onUnregister() override;

		virtual void onPrePhysicsTick(float deltaSeconds) override;
		virtual void onPostPhysicsTick(float deltaSeconds) override;

	private:
		badger::physics::Body* body = nullptr;

		// Maintain physics properties here and sync with badger Body.
		float invMass = 0.0f;

	};

}
