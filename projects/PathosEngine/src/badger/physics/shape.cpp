#include "shape.h"

// p : position, q : orientation
static vector3 rotatePoint(const vector3& p, const quat& q) {
	return vector3(glm::mat4_cast(q) * vector4(p, 1.0f));
}

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

	}
}
