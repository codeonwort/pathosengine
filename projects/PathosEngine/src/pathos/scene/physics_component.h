#pragma once

#include "badger/physics/shape.h"

#include "actor_component.h"

namespace pathos {

	class PhysicsComponent : public ActorComponent {

	public:
		PhysicsComponent();
		~PhysicsComponent();

	protected:
		virtual void onRegister() override;
		virtual void onUnregister() override;

		virtual void onPostPhysicsTick(float deltaSeconds) override;

	private:
		badger::physics::Body* body = nullptr;

	};

}
