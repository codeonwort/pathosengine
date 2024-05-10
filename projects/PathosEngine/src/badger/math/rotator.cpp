#include "rotator.h"
#include "glm/matrix.hpp"
#include "glm/gtx/euler_angles.hpp"

Rotator Rotator::directionToYawPitch(const vector3& dir)
{
	float mag = glm::length(dir);
	if (mag < 0.0001f) {
		return Rotator(0.0f, 0.0f, 0.0f);
	}

	// https://gamedev.stackexchange.com/questions/172147/convert-3d-direction-vectors-to-yaw-pitch-roll-angles
	const vector3 up(0.0f, 1.0f, 0.0f);
	const vector3 forward(1.0f, 0.0f, 0.0f);
	float yaw = atan2(-dir.z, dir.x);
	float pitch = asinf(dir.y / mag);
	float planeRightX = sin(yaw);
	float planeRightY = -cos(yaw);
	float roll = asin(up.x * planeRightX + up.z * planeRightY);
	if (up.y < 0.0f) roll = (roll < 0.0f ? -1.0f : 1.0f) * glm::pi<float>() - roll;
	return Rotator(glm::degrees(yaw), glm::degrees(pitch), glm::degrees(roll));
}

matrix4 Rotator::toMatrix() const {
	if (convention == RotatorConvention::YXZ) {
		return glm::eulerAngleYXZ(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));
	} else if (convention == RotatorConvention::ZYX) {
		return glm::eulerAngleZYX(glm::radians(roll), glm::radians(yaw), glm::radians(pitch));
	}
	CHECK_NO_ENTRY();
	return glm::eulerAngleXYZ(glm::radians(pitch), glm::radians(yaw), glm::radians(roll));
}

vector3 Rotator::toDirection() const {
	return matrix3(toMatrix()) * vector3(1.0f, 0.0f, 0.0f);
	//float theta = glm::radians(yaw);
	//float phi = glm::radians(pitch);
	//float cosPhi = ::cosf(phi);
	//return vector3(sinf(theta) * cosPhi, sinf(phi), cosf(theta) * cosPhi);
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
