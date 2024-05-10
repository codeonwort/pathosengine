#pragma once

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

// Rotation order
enum class RotatorConvention {
	YXZ, // yaw -> pitch -> roll
	ZYX, // roll -> yaw -> pitch
};

// Yaw-pitch-roll
struct Rotator {

	static Rotator directionToYawPitch(const vector3& dir);

	Rotator()
		: yaw(0.0f)
		, pitch(0.0f)
		, roll(0.0f)
		, convention(RotatorConvention::YXZ)
	{}

	Rotator(float inYaw, float inPitch, float inRoll, RotatorConvention inConvention = RotatorConvention::YXZ)
		: yaw(inYaw)
		, pitch(inPitch)
		, roll(inRoll)
		, convention(inConvention)
	{}

	matrix4 toMatrix() const;
	vector3 toDirection() const;

	void clampValues();

	float yaw;   // [-180, 180]
	float pitch; // [-90, 90]
	float roll;  // [-180, 180]

	RotatorConvention convention;

};
