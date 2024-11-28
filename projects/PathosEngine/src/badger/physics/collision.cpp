#include "collision.h"
#include "shape.h"

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

	}
}
