#include "physics_scene.h"

static const vector3 GRAVITY = vector3(0.0f, -9.8f, 0.0f);

namespace badger {
	namespace physics {

		void PhysicsScene::initialize() {
			//
		}

		void PhysicsScene::update(float deltaSeconds) {
			for (auto i = 0u; i < bodies.size(); ++i) {
				Body* body = bodies[i];

				// Gravity needs to be an impulse
				// I = dp, F = dp/dt => dp = F * dt => I = F * dt
				// F = mgs
				float mass = 1.0f / body->invMass;
				vector3 impulseGravity = GRAVITY * mass * deltaSeconds;
				body->applyImpulseLinear(impulseGravity);
			}

			for (auto i = 0u; i < bodies.size(); ++i) {
				bodies[i]->position += bodies[i]->linearVelocity * deltaSeconds;
			}
		}

		Body* PhysicsScene::allocateBody() {
			Body* body = new Body;
			bodies.push_back(body);
			return body;
		}

		void PhysicsScene::releaseBody(Body* body) {
			for (auto it = bodies.begin(); it != bodies.end(); ++it) {
				if (*it == body) {
					bodies.erase(it);
					return;
				}
			}
			CHECK_NO_ENTRY();
		}

	}
}
