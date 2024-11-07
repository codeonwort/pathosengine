#pragma once

#include "shape.h"

namespace badger {
	namespace physics {

		class Scene {

		public:
			void initialize();
			void update(float deltaSeconds);

		private:
			std::vector<Body> bodies;

		};

	}
}
