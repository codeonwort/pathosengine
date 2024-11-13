#pragma once

#include "shape.h"
#include <vector>

namespace badger {
	namespace physics {

		class PhysicsScene {

		public:
			void initialize();
			void update(float deltaSeconds);

			Body* allocateBody();
			void releaseBody(Body* body);

		private:
			// #todo-physics: Memory access is not cache friendly :(
			// Maybe import FreeNumberList from Cyseal and expose int32 handles rather than pointers?
			std::vector<Body*> bodies;

		};

	}
}
