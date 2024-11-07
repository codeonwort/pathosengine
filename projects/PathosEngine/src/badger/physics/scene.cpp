#include "scene.h"

static const vector3 GRAVITY = vector3(0.0f, -9.8f, 0.0f);

namespace badger {
	namespace physics {

		void Scene::initialize() {
			//
		}

		void Scene::update(float deltaSeconds) {
			// #todo-physics: Consider mass
			for (auto i = 0u; i < bodies.size(); ++i) {
				bodies[i].linearVelocity += GRAVITY * deltaSeconds;
			}

			for (auto i = 0u; i < bodies.size(); ++i) {
				bodies[i].position += bodies[i].linearVelocity * deltaSeconds;
			}
		}

	}
}
