#pragma once

#include "scene.h"
#include "badger/types/vector_types.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/fwd.hpp>
using quat = glm::quat;

namespace badger {
	namespace physics {

		class Shape {

		public:
			enum class EShapeType { Sphere };

			virtual EShapeType getType() const = 0;
			virtual vector3 getCenterOfMass() const { return centerOfMass; }

		protected:
			vector3 centerOfMass;

		};

		class ShapeSphere : public Shape {

		public:
			ShapeSphere(float inRadius) {
				radius = inRadius;
				centerOfMass = vector3(0.0f);
			}

			EShapeType getType() const override { return EShapeType::Sphere; }

		private:
			float radius;
		};

		class Body {
			friend class badger::physics::Scene;

		public:
			vector3 getCenterOfMassWorldSpace() const;
			vector3 getCenterOfMassModelSpace() const;

			vector3 worldSpaceToBodySpace(const vector3& position) const;
			vector3 bodySpaceToWorldSpace(const vector3& position) const;

		private:
			vector3 position = vector3(0.0f);
			quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			vector3 linearVelocity = vector3(0.0f);
			Shape* shape = nullptr;

		};

	}
}
