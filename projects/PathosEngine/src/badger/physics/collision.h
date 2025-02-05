#pragma once

#include "badger/types/vector_types.h"

#include <vector>

namespace badger {
	namespace physics {

		class Body;

		struct Contact {
			vector3 surfaceA_WS; // in world space
			vector3 surfaceB_WS;
			vector3 surfaceA_LS; // in local space
			vector3 surfaceB_LS;

			vector3 normal; // in world space
			float separationDistance; // positive when non-penetrating, negative otherwise
			float timeOfImpact;

			Body* bodyA;
			Body* bodyB;
		};

		struct CollisionPair {
			int32 a;
			int32 b;
			bool operator == (const CollisionPair& rhs) const {
				return ((a == rhs.a) && (b == rhs.b)) || ((a == rhs.b) && (b == rhs.a));
			}
			bool operator != (const CollisionPair& rhs) const {
				return !(*this == rhs);
			}
		};

		bool intersect(Body* bodyA, Body* bodyB, float dt, Contact& outContact);

		// Find all body pairs that might collide. Needs narrow phase to actually test it.
		void broadPhase(std::vector<Body*>& bodies, std::vector<CollisionPair>& outPairs, float deltaSeconds);

		// Intersection test between two convex shapes by Gilbert-Johnson-Keerthi algorithm.
		bool intersectGJK(const Body* bodyA, const Body* bodyB);

		// A variant that also writes the contact points to ptOnA and ptOnB.
		bool intersectGJK(const Body* bodyA, const Body* bodyB, float bias, vector3& ptOnA, vector3& ptOnB);

	}
}
