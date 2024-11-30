#include "physics_scene.h"
#include "collision.h"

#include <algorithm>

static const vector3 GRAVITY = vector3(0.0f, -9.8f, 0.0f);

namespace badger {
	namespace physics {

		static vector3 safeNormalize(const vector3& v) {
			if (v == vector3(0.0f)) return v;
			return glm::normalize(v);
		}

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

			// Calculate the impulse caused by friction.
			const float frictionA = bodyA->getFriction();
			const float frictionB = bodyB->getFriction();
			// #todo-physics: Approx. friction between two bodies
			const float friction = frictionA * frictionB;

			// The normal direction of the velocity w.r.t. the normal of the collision.
			vector3 velNorm = n * glm::dot(n, vAB);
			// The tangent direction of the velocity w.r.t. the normal of the collision.
			vector3 velTan = vAB - velNorm;

			// The tangential velocities relative to the other body.
			vector3 relVelTan = safeNormalize(velTan);

			const vector3 inertiaA = glm::cross(invWorldInertiaA * glm::cross(ra, relVelTan), ra);
			const vector3 inertiaB = glm::cross(invWorldInertiaB * glm::cross(rb, relVelTan), rb);
			const float invInertia = glm::dot(inertiaA + inertiaB, relVelTan);

			// Calculate the tangential impulse for friction.
			const float reducedMass = 1.0f / (invMassA + invMassB + invInertia);
			const vector3 impulseFriction = velTan * reducedMass * friction;

			// Apply kinetic friction.
			bodyA->applyImpulse(surfaceA, -impulseFriction);
			bodyB->applyImpulse(surfaceB, impulseFriction);

			// Move the objects to just outside of each other.
			if (contact.timeOfImpact == 0.0f) {
				float tA = invMassA / (invMassA + invMassB);
				float tB = invMassB / (invMassA + invMassB);
				vector3 ds = contact.surfaceB_WS - contact.surfaceA_WS;
				bodyA->setPosition(bodyA->getPosition() + ds * tA);
				bodyB->setPosition(bodyB->getPosition() - ds * tB);
			}
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

			// Broad phase
			std::vector<CollisionPair> collisionPairs;
			broadPhase(bodies, collisionPairs, deltaSeconds);

			// Narrow phase
			std::vector<Contact> contacts;
			contacts.reserve(bodies.size() * bodies.size());
#if 1
			for (const CollisionPair& cp : collisionPairs) {
				Body* bodyA = bodies[cp.a];
				Body* bodyB = bodies[cp.b];
				// Skip body pairs with infinite mass
				if (bodyA->hasInfiniteMass() && bodyB->hasInfiniteMass()) {
					continue;
				}

				Contact contact;
				if (intersect(bodyA, bodyB, deltaSeconds, contact)) {
					contacts.emplace_back(contact);
				}
			}
#else
			// Brute force ver.
			for (auto i = 0u; i < bodies.size(); ++i) {
				for (auto j = i + 1; j < bodies.size(); ++j) {
					Body* bodyA = bodies[i];
					Body* bodyB = bodies[j];
					// Skip body pairs with infinite mass
					if (bodyA->hasInfiniteMass() && bodyB->hasInfiniteMass()) {
						continue;
					}
					
					Contact contact;
					if (intersect(bodyA, bodyB, deltaSeconds, contact)) {
						contacts.emplace_back(contact);
					}
				}
			}
#endif

			// Sort by Time of Impact.
			if (contacts.size() > 1) {
				auto compareContacts = [](const Contact& a, const Contact& b) {
					return a.timeOfImpact < b.timeOfImpact;
				};
				std::sort(contacts.begin(), contacts.end(), compareContacts);
			}

			float accumulatedTime = 0.0f;
			for (auto i = 0u; i < contacts.size(); ++i) {
				Contact& contact = contacts[i];
				float dt = contact.timeOfImpact - accumulatedTime;

				Body* bodyA = contact.bodyA;
				Body* bodyB = contact.bodyB;
				// Skip body pairs with infinite mass
				if (bodyA->hasInfiniteMass() && bodyB->hasInfiniteMass()) {
					continue;
				}

				for (auto j = 0u; j < bodies.size(); ++j) {
					bodies[j]->update(dt);
				}

				resolveContact(contact);
				accumulatedTime += dt;
			}

			float timeRemaining = deltaSeconds - accumulatedTime;
			if (timeRemaining > 0.0f) {
				for (auto i = 0u; i < bodies.size(); ++i) {
					bodies[i]->update(timeRemaining);
				}
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
