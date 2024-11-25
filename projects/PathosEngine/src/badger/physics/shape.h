#pragma once

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/fwd.hpp"

using quat = glm::quat;

namespace badger {
	namespace physics {

		class Shape {

		public:
			enum class EShapeType { Sphere };

			virtual EShapeType getType() const = 0;
			virtual matrix3 inertiaTensor() const = 0;
			virtual vector3 getCenterOfMass() const { return centerOfMass; }

		protected:
			vector3 centerOfMass = vector3(0.0f);

		};

		class ShapeSphere : public Shape {

		public:
			ShapeSphere(float inRadius) {
				radius = inRadius;
				centerOfMass = vector3(0.0f);
			}

			EShapeType getType() const override { return EShapeType::Sphere; }
			matrix3 inertiaTensor() const override;

			inline float getRadius() const { return radius; }
			inline void setRadius(float value) { radius = value; }

		private:
			float radius;
		};

		class Body {
			friend class PhysicsScene;

		public:
			inline const Shape* getShape() const { return shape; }
			inline void setShape(Shape* inShape) { shape = inShape; }

			vector3 getCenterOfMassWorldSpace() const;
			vector3 getCenterOfMassModelSpace() const;

			vector3 worldSpaceToBodySpace(const vector3& position) const;
			vector3 bodySpaceToWorldSpace(const vector3& position) const;

			matrix3 getInverseInertiaTensorBodySpace() const;
			matrix3 getInverseInertiaTensorWorldSpace() const;

			inline vector3 getPosition() const { return position; }
			inline void setPosition(const vector3& inPosition) { position = inPosition; }

			inline float getInvMass() const { return invMass; }
			inline void setInvMass(float inInvMass) { invMass = inInvMass; }

			inline float getElasticity() const { return elasticity; }
			inline void setElasticity(float value) { elasticity = value; }

			inline vector3 getLinearVelocity() const { return linearVelocity; }
			inline void setLinearVelocity(const vector3& inVeocity) { linearVelocity = inVeocity; }

			inline vector3 getAngularVelocity() const { return angularVelocity; }

			void applyImpulse(const vector3& impulsePoint, const vector3& impulse);
			void applyImpulseLinear(const vector3& impulse);
			void applyImpulseAngular(const vector3& impulse);

			void update(float deltaSeconds);

		private:
			vector3 position = vector3(0.0f);
			quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			vector3 linearVelocity = vector3(0.0f);
			vector3 angularVelocity = vector3(0.0f);

			float invMass = 0.0f;
			float elasticity = 1.0f;
			Shape* shape = nullptr;

		};

	}
}
