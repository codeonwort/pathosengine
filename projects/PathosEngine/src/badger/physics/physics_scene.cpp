#include "physics_scene.h"
#include "collision.h"

static const vector3 GRAVITY = vector3(0.0f, -9.8f, 0.0f);

namespace badger {
	namespace physics {

		static void resolveContact(Contact& contact) {
			Body* bodyA = contact.bodyA;
			Body* bodyB = contact.bodyB;

			const vector3 surfaceA = contact.surfaceA_WS;
			const vector3 surfaceB = contact.surfaceB_WS;
			const float invMassA = bodyA->getInvMass();
			const float invMassB = bodyB->getInvMass();

			const float elasticityA = bodyA->getElasticity();
			const float elasticityB = bodyB->getElasticity();
			// #todo-physics: Approx. elasticity between two bodies
			const float elasticity = elasticityA * elasticityB;

			const matrix3 invWorldInertiaA = bodyA->getInverseInertiaTensorWorldSpace();
			const matrix3 invWorldInertiaB = bodyB->getInverseInertiaTensorWorldSpace();

			const vector3 n = contact.normal;
			const vector3 ra = surfaceA - bodyA->getCenterOfMassWorldSpace();
			const vector3 rb = surfaceB - bodyB->getCenterOfMassWorldSpace();

			const vector3 angularJA = glm::cross(invWorldInertiaA * glm::cross(ra, n), ra);
			const vector3 angularJB = glm::cross(invWorldInertiaB * glm::cross(rb, n), rb);
			const float angularFactor = glm::dot(angularJA + angularJB, n);

			// World space velocity of the motion and rotation.
			const vector3 velA = bodyA->getLinearVelocity() + glm::cross(bodyA->getAngularVelocity(), ra);
			const vector3 velB = bodyB->getLinearVelocity() + glm::cross(bodyB->getAngularVelocity(), rb);

			// Calculate the collision impulse.
			vector3 vAB = velA - velB;
			float impulseJ = (1.0f + elasticity) * glm::dot(vAB, n) / (invMassA + invMassB + angularFactor);
			vector3 vectorImpulseJ = n * impulseJ;

			bodyA->applyImpulse(surfaceA, -vectorImpulseJ);
			bodyB->applyImpulse(surfaceB, vectorImpulseJ);

			// Move the objects to just outside of each other.
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
