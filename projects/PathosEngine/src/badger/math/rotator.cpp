#include "rotator.h"
#include "glm/matrix.hpp"
#include "glm/gtx/euler_angles.hpp"

Rotator Rotator::directionToYawPitch(const vector3& dir)
{
	float mag = glm::length(dir);
	if (mag < 0.0001f) {
		return Rotator(0.0f, 0.0f, 0.0f);
	}

	// #todo-spline: Wrong
	float yaw = glm::degrees(atan2(-dir.z, dir.x));
	//float pitch = glm::degrees(asinf(dir.y / mag));
	float pitch = glm::degrees(atan(sqrtf(dir.x * dir.x + dir.z * dir.z) / dir.y));
	return Rotator(yaw, pitch, 0.0f);
}

matrix4 Rotator::toMatrix() const {
	// #todo-spline: Is this right?
	return glm::eulerAngleYXZ(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));
}

void Rotator::clampValues() {
	yaw = fmod(yaw, 360.0f);
	pitch = fmod(pitch, 360.0f);
	roll = fmod(roll, 360.0f);

	if (yaw < -180.0f) yaw += 360.0f;
	if (pitch < -180.0f) pitch += 360.0f;
	if (roll < -180.0f) roll += 360.0f;

	//yaw = clamp(-180.0f, yaw, 180.0f);
	//pitch = clamp(-90.0f, yaw, 90.0f);
	//roll = clamp(-180.0f, roll, 180.0f);
}
