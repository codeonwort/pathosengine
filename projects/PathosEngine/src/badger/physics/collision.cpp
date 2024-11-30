#include "collision.h"
#include "shape.h"

#include <algorithm>

namespace badger {
	namespace physics {

		static vector3 safeNormalize(const vector3& v) {
			if (v == vector3(0.0f)) return v;
			return glm::normalize(v);
		}

		static bool raySphere(
			const vector3& rayStart, const vector3& rayDir,
			const vector3& sphereCenter, float sphereRadius,
			float& t1, float& t2)
		{
			vector3 m = sphereCenter - rayStart;
			float a = glm::dot(rayDir, rayDir);
			float b = glm::dot(m, rayDir);
			float c = glm::dot(m, m) - sphereRadius * sphereRadius;

			float delta = b * b - a * c;
			float invA = 1.0f / a;

			if (delta < 0.0f) return false; // No real solutions exist

			float deltaRoot = sqrtf(delta);
			t1 = invA * (b - deltaRoot);
			t2 = invA * (b + deltaRoot);

			return true;
		}

		static bool sphereSphereDynamic(
			const ShapeSphere* shapeA, const ShapeSphere* shapeB,
			const vector3& posA, const vector3& posB,
			const vector3& velA, const vector3& velB,
			float dt,
			vector3& surfaceA, vector3& surfaceB, float& toi)
		{
			const vector3 relVel = velA - velB;
			const vector3 startA = posA;
			const vector3 endA = posA + relVel * dt;
			const vector3 rayDir = endA - startA;

			float t0 = 0.0f, t1 = 0.0f;
			if (glm::dot(rayDir, rayDir) < 0.001f * 0.001f) {
				// Ray is too short, just check if already intersecting
				vector3 ab = posB - posA;
				float radius = shapeA->getRadius() + shapeB->getRadius() + 0.001f;
				if (glm::dot(ab, ab) > radius * radius) {
					return false;
				}
			} else if (!raySphere(posA, rayDir, posB, shapeA->getRadius() + shapeB->getRadius(), t0, t1)) {
				return false;
			}

			// [0, 1] -> [0, dt]
			t0 *= dt; t1 *= dt;

			// If the collision is only in the past, then there's not future collision this frame
			if (t1 < 0.0f) return false;

			toi = t0 < 0.0f ? 0.0f : t0;

			// If the earliest collision is too far in the future, then there's no collision this frame
			if (toi > dt) return false;

			vector3 newPosA = posA + velA * toi;
			vector3 newPosB = posB + velB * toi;
			vector3 ab = safeNormalize(newPosB - newPosA);

			surfaceA = newPosA + ab * shapeA->getRadius();
			surfaceB = newPosB - ab * shapeB->getRadius();
			return true;
		}

		bool intersect(Body* bodyA, Body* bodyB, float dt, Contact& outContact) {
			outContact.bodyA = bodyA;
			outContact.bodyB = bodyB;

			if (bodyA->getShape()->getType() == Shape::EShapeType::Sphere && bodyB->getShape()->getType() == Shape::EShapeType::Sphere) {
				const ShapeSphere* sphereA = (const ShapeSphere*)bodyA->getShape();
				const ShapeSphere* sphereB = (const ShapeSphere*)bodyB->getShape();

				vector3 posA = bodyA->getPosition();
				vector3 velA = bodyA->getLinearVelocity();
				vector3 posB = bodyB->getPosition();
				vector3 velB = bodyB->getLinearVelocity();

				if (sphereSphereDynamic(sphereA, sphereB, posA, posB, velA, velB, dt,
					outContact.surfaceA_WS, outContact.surfaceB_WS, outContact.timeOfImpact))
				{
					// Step bodies forward to get local space collision points
					bodyA->update(outContact.timeOfImpact);
					bodyB->update(outContact.timeOfImpact);

					outContact.surfaceA_LS = bodyA->worldSpaceToBodySpace(outContact.surfaceA_WS);
					outContact.surfaceB_LS = bodyB->worldSpaceToBodySpace(outContact.surfaceB_WS);

					outContact.normal = safeNormalize(bodyA->getPosition() - bodyB->getPosition());

					// Unwind time step
					bodyA->update(-outContact.timeOfImpact);
					bodyB->update(-outContact.timeOfImpact);

					vector3 ab = bodyB->getPosition() - bodyA->getPosition();
					float r = glm::length(ab) - (sphereA->getRadius() + sphereB->getRadius());

					outContact.separationDistance = r;
					return true;
				}
			}

			return false;
		}

		static void sortBodyBounds(std::vector<Body*>& bodies, std::vector<PseudoBody>& sortedArray, float deltaSeconds) {
			auto compareSAP = [](const PseudoBody& a, const PseudoBody& b) {
				return a.value < b.value;
			};
			vector3 axis = glm::normalize(vector3(1.0f, 1.0f, 1.0f));

			sortedArray.resize(bodies.size() * 2u);
			for (auto i = 0u; i < bodies.size(); ++i) {
				const Body* body = bodies[i];
				AABB bounds = body->getShape()->getBounds(body->getPosition(), body->getOrientation());

				vector3 mov = body->getLinearVelocity() * deltaSeconds;
				bounds = bounds + AABB::fromMinMax(bounds.minBounds + mov, bounds.maxBounds + mov);

				const vector3 EPS(0.01f);
				bounds = bounds + AABB::fromMinMax(bounds.minBounds - EPS, bounds.maxBounds + EPS);

				sortedArray[i * 2 + 0].id = (int32)i;
				sortedArray[i * 2 + 0].value = glm::dot(axis, bounds.minBounds);
				sortedArray[i * 2 + 0].isMin = true;

				sortedArray[i * 2 + 1].id = (int32)i;
				sortedArray[i * 2 + 1].value = glm::dot(axis, bounds.maxBounds);
				sortedArray[i * 2 + 1].isMin = false;
			}

			std::sort(sortedArray.begin(), sortedArray.end(), compareSAP);
		}

		static void buildPairs(std::vector<CollisionPair>& collisionPairs, const std::vector<PseudoBody>& sortedBodies) {
			collisionPairs.clear();
			collisionPairs.reserve(sortedBodies.size() * sortedBodies.size() / 4);

			for (auto i = 0u; i < sortedBodies.size(); ++i) {
				const PseudoBody& a = sortedBodies[i];
				if (!a.isMin) continue;

				CollisionPair pair;
				pair.a = a.id;

				for (int j = i + 1; j < sortedBodies.size(); ++j) {
					const PseudoBody& b = sortedBodies[j];
					if (b.id == a.id) break;
					if (b.isMin == false) continue;

					pair.b = b.id;
					collisionPairs.emplace_back(pair);
				}
			}
		}

		static void sweepAndPrune1D(std::vector<Body*>& bodies, std::vector<CollisionPair>& outPairs, float deltaSeconds) {
			std::vector<PseudoBody> sortedBodies;
			sortBodyBounds(bodies, sortedBodies, deltaSeconds);
			buildPairs(outPairs, sortedBodies);
		}

		void broadPhase(std::vector<Body*>& bodies, std::vector<CollisionPair>& outPairs, float deltaSeconds) {
			sweepAndPrune1D(bodies, outPairs, deltaSeconds);
		}

	}
}
