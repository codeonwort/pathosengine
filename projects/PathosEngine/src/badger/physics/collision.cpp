#include "collision.h"
#include "shape.h"

namespace badger {
	namespace physics {

		bool intersect(const Body* bodyA, const Body* bodyB, Contact& outContact) {
			outContact.bodyA = const_cast<Body*>(bodyA);
			outContact.bodyB = const_cast<Body*>(bodyB);

			vector3 ab = bodyB->getPosition() - bodyA->getPosition();
			outContact.normal = glm::normalize(ab);

			// #todo-physics: Assumes sphere
			const ShapeSphere* sphereA = (const ShapeSphere*)bodyA->getShape();
			const ShapeSphere* sphereB = (const ShapeSphere*)bodyB->getShape();

			outContact.surfaceA_WS = bodyA->getPosition() + outContact.normal * sphereA->getRadius();
			outContact.surfaceB_WS = bodyB->getPosition() - outContact.normal * sphereB->getRadius();

			float radiusAB = sphereA->getRadius() + sphereB->getRadius();
			float lengthSq = glm::dot(ab, ab);
			return lengthSq <= (radiusAB * radiusAB);
		}

	}
}
