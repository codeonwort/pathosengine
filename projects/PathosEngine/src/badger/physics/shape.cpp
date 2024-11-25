#include "shape.h"

#include "glm/gtx/quaternion.hpp"

// #todo-physics: Hard-coded max angular speed
static const float MAX_ANGULAR_SPEED = 30.0f;

// p : position, q : orientation
static vector3 rotatePoint(const vector3& p, const quat& q) {
	return vector3(glm::mat4_cast(q) * vector4(p, 1.0f));
}

namespace badger {
	
	// ShapeSphere
	namespace physics {

		matrix3 ShapeSphere::inertiaTensor() const {
			matrix3 tensor(0.0f);
			tensor[0][0] = 2.0f * radius * radius / 5.0f;
			tensor[1][1] = 2.0f * radius * radius / 5.0f;
			tensor[2][2] = 2.0f * radius * radius / 5.0f;
			return tensor;
		}

	} // end of physics (ShapeSphere)

	// Body
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
			matrix3 tensor = shape->inertiaTensor();
			matrix3 invTensor = glm::inverse(tensor) * invMass;
			return invTensor;
		}

		matrix3 Body::getInverseInertiaTensorWorldSpace() const {
			matrix3 tensor = shape->inertiaTensor();
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
			matrix3 inertiaTensor = orient * shape->inertiaTensor() * glm::transpose(orient);
			vector3 alpha = glm::inverse(inertiaTensor) * glm::cross(angularVelocity, inertiaTensor * angularVelocity);
			angularVelocity += alpha * deltaSeconds;

			// Update orientation
			vector3 dAngle = angularVelocity * deltaSeconds;
			quat dq = quat(glm::length(dAngle), dAngle);
			orientation = glm::normalize(dq * orientation);

			position = positionCM + rotatePoint(cmToPos, dq);
		}

	} // end of physics (Body)

} // end of badger
