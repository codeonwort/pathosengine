#include "physics_scene.h"

static const vector3 GRAVITY = vector3(0.0f, -9.8f, 0.0f);

namespace badger {
	namespace physics {

		void PhysicsScene::initialize() {
			//
		}

		void PhysicsScene::update(float deltaSeconds) {
			// #todo-physics: Consider mass
			for (auto i = 0u; i < bodies.size(); ++i) {
				bodies[i]->linearVelocity += GRAVITY * deltaSeconds;
			}

			for (auto i = 0u; i < bodies.size(); ++i) {
				bodies[i]->position += bodies[i]->linearVelocity * deltaSeconds;
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
