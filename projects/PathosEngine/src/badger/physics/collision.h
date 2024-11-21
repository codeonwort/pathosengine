#pragma once

#include "badger/types/vector_types.h"

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

		bool intersect(const Body* bodyA, const Body* bodyB, Contact& outContact);

	}
}
