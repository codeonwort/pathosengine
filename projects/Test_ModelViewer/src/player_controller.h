#pragma once

#include "pathos/scene/actor.h"
using namespace pathos;

// #todo: Eh... it's time to make this an engine-level class I guess?
//        Same controller code exist in every application projects...
class PlayerController : public Actor {

public:
	virtual void onSpawn() override;
	virtual void onTick(float deltaSeconds) override;

public:
	// movement (meters/sec)
	float speedRight = 2.0f;
	float speedForward = 2.0f;
	float speedUp = 2.0f;

	// rotation (degrees/sec)
	float speedYaw = 120.0f;
	float speedPitch = 120.0f;

private:
	void setupInput();

	bool rotateByMouse = false;

};
