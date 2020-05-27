#include "rotator.h"
#include "glm/matrix.hpp"
#include "glm/gtx/euler_angles.hpp"

matrix4 Rotator::toMatrix() const {
	return glm::eulerAngleYXZ(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));
}

void Rotator::clampValues() {
	yaw = clamp(-180.0f, yaw, 180.0f);
	pitch = clamp(-90.0f, yaw, 90.0f);
	roll = clamp(-180.0f, roll, 180.0f);
}
