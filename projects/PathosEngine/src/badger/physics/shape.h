#pragma once

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include "badger/math/aabb.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/fwd.hpp"

#include <vector>

using quat = glm::quat;

namespace badger {
	namespace physics {

		class Shape {

		public:
			enum class EShapeType { Sphere, Box, Convex };

			virtual void build(const std::vector<vector3>& points) {}

			// - Needed for collision detection between general convex shapes
			//   and for continuous collision detection between convex shapes.
			// - 'dir' should be a unit vector.
			virtual vector3 support(const vector3& dir, const vector3& pos, const quat& orient, float bias) const = 0;

			virtual AABB getBounds(const vector3& pos, const quat& orient) const = 0;
			virtual AABB getBounds() const = 0;

			virtual EShapeType getType() const = 0;
			virtual matrix3 getInertiaTensor() const = 0;
			virtual vector3 getCenterOfMass() const { return centerOfMass; }

			virtual float fastestLinearSpeed(const vector3& angularVelocity, const vector3& dir) const { return 0.0f; }

		protected:
			vector3 centerOfMass = vector3(0.0f);

		};

		class ShapeSphere : public Shape {

		public:
			ShapeSphere(float inRadius) {
				radius = inRadius;
				centerOfMass = vector3(0.0f);
			}
			
			vector3 support(const vector3& dir, const vector3& pos, const quat& orient, float bias) const override;

			AABB getBounds(const vector3& pos, const quat& orient) const override;
			AABB getBounds() const override;

			EShapeType getType() const override { return EShapeType::Sphere; }
			matrix3 getInertiaTensor() const override;

			inline float getRadius() const { return radius; }
			inline void setRadius(float value) { radius = value; }

		private:
			float radius;
		};

		class ShapeBox : public Shape {

		public:
			explicit ShapeBox(const std::vector<vector3>& inPoints) {
				build(inPoints);
			}
			explicit ShapeBox(const vector3& extents) {
				buildSub(AABB::fromCenterAndHalfSize(vector3(0.0f), 0.5f * extents));
			}

			void build(const std::vector<vector3>& points) override;

			vector3 support(const vector3& dir, const vector3& pos, const quat& orient, float bias) const override;

			AABB getBounds(const vector3& pos, const quat& orient) const override;
			AABB getBounds() const override;

			EShapeType getType() const override { return EShapeType::Box; }
			matrix3 getInertiaTensor() const override;

			float fastestLinearSpeed(const vector3& angularVelocity, const vector3& dir) const override;

		private:
			void buildSub(const AABB& aabb);

			std::vector<vector3> points;
			AABB bounds;
		};

		class ShapeConvex : public Shape {

		public:
			explicit ShapeConvex(const std::vector<vector3>& points) {
				build(points);
			}

			void build(const std::vector<vector3>& points) override;

			vector3 support(const vector3& dir, const vector3& pos, const quat& orient, float bias) const override;

			AABB getBounds(const vector3& pos, const quat& orient) const override;
			AABB getBounds() const override { return bounds; }

			EShapeType getType() const override { return EShapeType::Convex; }
			matrix3 getInertiaTensor() const override { return inertiaTensor; }

			float fastestLinearSpeed(const vector3& angularVelocity, const vector3& dir) const override;

		private:
			std::vector<vector3> shapePoints;
			AABB bounds;
			matrix3 inertiaTensor;

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

			inline quat getOrientation() const { return orientation; }

			inline float getInvMass() const { return invMass; }
			inline void setInvMass(float inInvMass) { invMass = inInvMass; }
			inline bool hasInfiniteMass() const { return invMass == 0.0f; }

			inline float getElasticity() const { return elasticity; }
			inline void setElasticity(float value) { elasticity = value; }

			inline float getFriction() const { return friction; }
			inline void setFriction(float value) { friction = value; }

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
			float friction = 1.0f;
			Shape* shape = nullptr;

		};

	}
}
