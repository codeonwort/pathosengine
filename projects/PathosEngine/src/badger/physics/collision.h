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

		struct PseudoBody {
			int32 id;
			float value;
			bool isMin;
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

		void broadPhase(std::vector<Body*>& bodies, std::vector<CollisionPair>& outPairs, float deltaSeconds);

		bool intersectGJK(const Body* bodyA, const Body* bodyB);

	}
}
