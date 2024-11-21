#include "physics_scene.h"
#include "collision.h"

static const vector3 GRAVITY = vector3(0.0f, -9.8f, 0.0f);

namespace badger {
	namespace physics {

		static void resolveContact(Contact& contact) {
			Body* bodyA = contact.bodyA;
			Body* bodyB = contact.bodyB;

			bodyA->setLinearVelocity(vector3(0.0f));
			bodyB->setLinearVelocity(vector3(0.0f));
		}

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

			// Check for collisions between bodies
			for (int i = 0; i < bodies.size(); ++i) {
				for (int j = i + 1; j < bodies.size(); ++j) {
					Body* bodyA = bodies[i];
					Body* bodyB = bodies[j];
					// Skip body pairs with infinite mass
					if (bodyA->invMass == 0.0f && bodyB->invMass == 0.0f) {
						continue;
					}
					
					Contact contact;
					if (intersect(bodyA, bodyB, contact)) {
						resolveContact(contact);
					}
				}
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
