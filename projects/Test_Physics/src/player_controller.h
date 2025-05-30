#pragma once

#include "pathos/scene/actor.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

class PlayerController : public Actor {

public:
	virtual void onSpawn() override;
	virtual void onTick(float deltaSeconds) override;

	inline void setControlTarget(sharedPtr<Actor> inTarget) { target = inTarget; }

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

	sharedPtr<Actor> target;

};
