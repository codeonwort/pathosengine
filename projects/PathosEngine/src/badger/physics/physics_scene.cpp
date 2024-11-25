#include "physics_scene.h"
#include "collision.h"

static const vector3 GRAVITY = vector3(0.0f, -9.8f, 0.0f);

namespace badger {
	namespace physics {

		static void resolveContact(Contact& contact) {
			Body* bodyA = contact.bodyA;
			Body* bodyB = contact.bodyB;

			float invMassA = bodyA->getInvMass();
			float invMassB = bodyB->getInvMass();
			float elasticityA = bodyA->getElasticity();
			float elasticityB = bodyB->getElasticity();

			// #todo-physics: Approx. elasticity between two bodies
			float elasticity = elasticityA * elasticityB;

			// Calculate the collision impulse
			const vector3& n = contact.normal;
			vector3 vAB = bodyA->getLinearVelocity() - bodyB->getLinearVelocity();
			float impulseJ = -(1.0f + elasticity) * glm::dot(vAB, n) / (invMassA + invMassB);
			vector3 vectorImpulseJ = n * impulseJ;

			bodyA->applyImpulseLinear(vectorImpulseJ);
			bodyB->applyImpulseLinear(-vectorImpulseJ);

			// Move the objects to just outside of each other
			float tA = invMassA / (invMassA + invMassB);
			float tB = invMassB / (invMassA + invMassB);

			vector3 ds = contact.surfaceB_WS - contact.surfaceA_WS;
			bodyA->setPosition(bodyA->getPosition() + ds * tA);
			bodyB->setPosition(bodyB->getPosition() - ds * tB);
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
				bodies[i]->update(deltaSeconds);
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
