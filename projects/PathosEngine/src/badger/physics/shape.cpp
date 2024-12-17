#include "shape.h"
#include "badger/assertion/assertion.h"

#include "glm/gtx/quaternion.hpp"

// #todo-physics: Hard-coded max angular speed
static const float MAX_ANGULAR_SPEED = 30.0f;

// p : position, q : orientation
static vector3 rotatePoint(const vector3& p, const quat& q) {
	return vector3(glm::mat4_cast(q) * vector4(p, 1.0f));
}

static AABB buildAABB(const std::vector<vector3>& points) {
	CHECK(points.size() > 0);
	AABB aabb = AABB::fromMinMax(points[0], points[0]);
	if (points.size() > 1) {
		for (auto it = points.begin() + 1; it != points.end(); ++it) {
			aabb.expand(*it);
		}
	}
	return aabb;
}

// ShapeSphere
namespace badger {
	namespace physics {

		vector3 ShapeSphere::support(const vector3& dir, const vector3& pos, const quat& orient, float bias) const {
			return pos + dir * (radius + bias);
		}

		AABB ShapeSphere::getBounds(const vector3& pos, const quat& orient) const {
			return AABB::fromMinMax(vector3(-radius) + pos, vector3(radius) + pos);
		}

		AABB ShapeSphere::getBounds() const {
			return AABB::fromMinMax(vector3(-radius), vector3(radius));
		}

		matrix3 ShapeSphere::getInertiaTensor() const {
			matrix3 tensor(0.0f);
			tensor[0][0] = 2.0f * radius * radius / 5.0f;
			tensor[1][1] = 2.0f * radius * radius / 5.0f;
			tensor[2][2] = 2.0f * radius * radius / 5.0f;
			return tensor;
		}

	}
}

// ShapeBox
namespace badger {
	namespace physics {

		void ShapeBox::build(const std::vector<vector3>& inPoints) {
			bounds = buildAABB(inPoints);

			points.clear();
			points.push_back(vector3(bounds.minBounds.x, bounds.minBounds.y, bounds.minBounds.z));
			points.push_back(vector3(bounds.maxBounds.x, bounds.minBounds.y, bounds.minBounds.z));
			points.push_back(vector3(bounds.minBounds.x, bounds.maxBounds.y, bounds.minBounds.z));
			points.push_back(vector3(bounds.minBounds.x, bounds.minBounds.y, bounds.maxBounds.z));
			points.push_back(vector3(bounds.maxBounds.x, bounds.maxBounds.y, bounds.maxBounds.z));
			points.push_back(vector3(bounds.minBounds.x, bounds.maxBounds.y, bounds.maxBounds.z));
			points.push_back(vector3(bounds.maxBounds.x, bounds.minBounds.y, bounds.maxBounds.z));
			points.push_back(vector3(bounds.maxBounds.x, bounds.maxBounds.y, bounds.minBounds.z));

			centerOfMass = bounds.getCenter();
		}

		vector3 ShapeBox::support(const vector3& dir, const vector3& pos, const quat& orient, float bias) const {
			// Find the point in furthest in direction.
			vector3 maxPt = rotatePoint(points[0], orient) + pos;
			float maxDist = glm::dot(dir, maxPt);
			for (auto i = 0u; i < points.size(); ++i) {
				const vector3 pt = rotatePoint(points[i], orient) + pos;
				float dist = glm::dot(dir, pt);
				if (dist > maxDist) {
					maxDist = dist;
					maxPt = pt;
				}
			}
			vector3 norm = bias * glm::normalize(dir);
			return maxPt + norm;
		}

		AABB ShapeBox::getBounds(const vector3& pos, const quat& orient) const {
			std::vector<vector3> corners = {
				vector3(bounds.minBounds.x, bounds.minBounds.y, bounds.minBounds.z),
				vector3(bounds.maxBounds.x, bounds.minBounds.y, bounds.minBounds.z),
				vector3(bounds.minBounds.x, bounds.maxBounds.y, bounds.minBounds.z),
				vector3(bounds.minBounds.x, bounds.minBounds.y, bounds.maxBounds.z),
				vector3(bounds.maxBounds.x, bounds.maxBounds.y, bounds.maxBounds.z),
				vector3(bounds.minBounds.x, bounds.maxBounds.y, bounds.maxBounds.z),
				vector3(bounds.maxBounds.x, bounds.minBounds.y, bounds.maxBounds.z),
				vector3(bounds.maxBounds.x, bounds.maxBounds.y, bounds.minBounds.z),
			};
			for (int32 i = 0; i < 8; ++i) {
				corners[i] = rotatePoint(corners[i], orient) + pos;
			}
			return buildAABB(corners);
		}

		AABB ShapeBox::getBounds() const {
			return bounds;
		}

		matrix3 ShapeBox::getInertiaTensor() const {
			// Inertia tensor for box centered around zero.
			float dx = bounds.maxBounds.x - bounds.minBounds.x;
			float dy = bounds.maxBounds.y - bounds.minBounds.y;
			float dz = bounds.maxBounds.z - bounds.minBounds.z;

			matrix3 tensor = matrix3(0.0f);
			tensor[0][0] = (dy * dy + dz * dz) / 12.0f;
			tensor[1][1] = (dx * dx + dz * dz) / 12.0f;
			tensor[2][2] = (dx * dx + dy * dy) / 12.0f;

			// Use the Parallel Axis theorem to get the inertia tensor for a box
			// that is not centered around the origin.

			vector3 cm = bounds.getCenter();
			vector3 R = vector3(0.0f) - cm; // center of mass to the origin
			float R2 = glm::dot(R, R);
			matrix3 patTensor;
			patTensor[0] = vector3(R2 - R.x * R.x, R.x * R.y, R.x * R.z);
			patTensor[1] = vector3(R.y * R.x, R2 - R.y * R.y, R.y * R.z);
			patTensor[2] = vector3(R.z * R.x, R.z * R.y, R2 - R.z * R.z);

			// Add the center of mass tensor and the Parallel Axis theorm tensor together.
			tensor += patTensor;
			return tensor;
		}

		float ShapeBox::fastestLinearSpeed(const vector3& angularVelocity, const vector3& dir) const {
			float maxSpeed = 0.0f;
			for (auto i = 0u; i < points.size(); ++i) {
				vector3 r = points[i] - centerOfMass;
				vector3 linearVelocity = glm::cross(angularVelocity, r);
				float speed = glm::dot(dir, linearVelocity);
				if (speed > maxSpeed) {
					maxSpeed = speed;
				}
			}
			return maxSpeed;
		}

	}
}

// ShapeConvex
namespace badger {
	namespace physics {
		
		void ShapeConvex::build(const std::vector<vector3>& points) {
			// WIP: p.10
		}

		vector3 ShapeConvex::support(const vector3& dir, const vector3& pos, const quat& orient, float bias) const {

		}

		AABB ShapeConvex::getBounds(const vector3& pos, const quat& orient) const {
			std::vector<vector3> corners = {
				vector3(bounds.minBounds.x, bounds.minBounds.y, bounds.minBounds.z),
				vector3(bounds.maxBounds.x, bounds.minBounds.y, bounds.minBounds.z),
				vector3(bounds.minBounds.x, bounds.maxBounds.y, bounds.minBounds.z),
				vector3(bounds.minBounds.x, bounds.minBounds.y, bounds.maxBounds.z),
				vector3(bounds.maxBounds.x, bounds.maxBounds.y, bounds.maxBounds.z),
				vector3(bounds.minBounds.x, bounds.maxBounds.y, bounds.maxBounds.z),
				vector3(bounds.maxBounds.x, bounds.minBounds.y, bounds.maxBounds.z),
				vector3(bounds.maxBounds.x, bounds.maxBounds.y, bounds.minBounds.z),
			};
			for (int32 i = 0; i < 8; ++i) {
				corners[i] = rotatePoint(corners[i], orient) + pos;
			}
			return buildAABB(corners);
		}

		AABB ShapeConvex::getBounds() const {

		}

		matrix3 ShapeConvex::getInertiaTensor() const {

		}

		float ShapeConvex::fastestLinearSpeed(const vector3& angularVelocity, const vector3& dir) const {
			float maxSpeed = 0.0f;
			for (const vector3& pt : points) {
				vector3 r = pt - centerOfMass;
				vector3 linearVelocity = glm::cross(angularVelocity, r);
				float speed = glm::dot(dir, linearVelocity);
				if (speed > maxSpeed) maxSpeed = speed;
			}
			return maxSpeed;
		}

	}
}

// Body
namespace badger {
	namespace physics {

		vector3 Body::getCenterOfMassWorldSpace() const {
			vector3 centerOfMass = shape->getCenterOfMass();
			vector3 pos = position + rotatePoint(centerOfMass, orientation);
			return pos;
		}

		vector3 Body::getCenterOfMassModelSpace() const {
			vector3 centerOfMass = shape->getCenterOfMass();
			return centerOfMass;
		}

		vector3 Body::worldSpaceToBodySpace(const vector3& position) const {
			vector3 tmp = position - getCenterOfMassWorldSpace();
			quat inverseOrient = glm::inverse(orientation);
			vector3 bodySpace = rotatePoint(tmp, inverseOrient);
			return bodySpace;
		}

		vector3 Body::bodySpaceToWorldSpace(const vector3& position) const {
			vector3 worldSpace = getCenterOfMassWorldSpace() + rotatePoint(position, orientation);
			return worldSpace;
		}

		matrix3 Body::getInverseInertiaTensorBodySpace() const {
			matrix3 tensor = shape->getInertiaTensor();
			matrix3 invTensor = glm::inverse(tensor) * invMass;
			return invTensor;
		}

		matrix3 Body::getInverseInertiaTensorWorldSpace() const {
			matrix3 tensor = shape->getInertiaTensor();
			matrix3 invTensor = glm::inverse(tensor) * invMass;
			matrix3 orient = glm::toMat3(orientation);
			invTensor = orient * invTensor * glm::transpose(orient);
			return invTensor;
		}

		void Body::applyImpulse(const vector3& impulsePoint, const vector3& impulse) {
			if (invMass == 0.0f) return;

			// impulsePoint is the world space location of the application of the impulse.
			// impulse is the world space direction and magnitude of the impulse.
			applyImpulseLinear(impulse);

			vector3 position = getCenterOfMassWorldSpace(); // applying impulses must produce torques through the center of mass
			vector3 r = impulsePoint - position;
			vector3 dL = glm::cross(r, impulse); // in world space
			applyImpulseAngular(dL);
		}

		void Body::applyImpulseLinear(const vector3& impulse) {
			if (0.0f == invMass) {
				return;
			}

			// p = mv (p: momentum)
			// dp = m dv = J
			// dv = J / m
			linearVelocity += impulse * invMass;
		}

		void Body::applyImpulseAngular(const vector3& impulse) {
			if (invMass == 0.0f) return;

			// L = I w = r x p
			// dL = I dw = r x J
			// => dw = I^-1 * ( r x J )
			angularVelocity += getInverseInertiaTensorWorldSpace() * impulse;
			if (glm::length2(angularVelocity) > MAX_ANGULAR_SPEED * MAX_ANGULAR_SPEED) {
				angularVelocity = glm::normalize(angularVelocity);
				angularVelocity *= MAX_ANGULAR_SPEED;
			}
		}

		void Body::update(float deltaSeconds) {
			position += linearVelocity * deltaSeconds;

			vector3 positionCM = getCenterOfMassWorldSpace();
			vector3 cmToPos = position - positionCM;

			// Total torque is equal to external applied torques + internal torque (precession)
			// T = T_external + omega x I * omega
			// T_external = 0 because it was applied in the collision response function
			// T = Ia = w x I * w
			// a = I^-1 ( w x I * w)
			matrix3 orient = glm::toMat3(orientation);
			matrix3 inertiaTensor = orient * shape->getInertiaTensor() * glm::transpose(orient);
			vector3 alpha = glm::inverse(inertiaTensor) * glm::cross(angularVelocity, inertiaTensor * angularVelocity);
			angularVelocity += alpha * deltaSeconds;

			// Update orientation
			vector3 dAngle = angularVelocity * deltaSeconds;
			quat dq = quat(glm::length(dAngle), dAngle);
			orientation = glm::normalize(dq * orientation);

			position = positionCM + rotatePoint(cmToPos, dq);
		}

	}
}
