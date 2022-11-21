#pragma once

#include "pathos/scene/actor.h"
using namespace pathos;

class PlayerController : public Actor
{

public:
	virtual void onSpawn() override;
	virtual void onTick(float deltaSeconds) override;

	void setPlayerPawn(Actor* player);
	void togglePhotoMode();

	float cameraHeightOffset  = 1.8f;
	float cameraForwardOffset = 3.0f;

private:
	void setupInput();
	void tickGameplay(float deltaSeconds);
	void tickPhotoMode(float deltaSeconds);

	Actor* playerPawn = nullptr;
	bool inPhotoMode = false;

	// gameplay
	Rotator pawnRotation;
	float linearSpeed = 0.0f;

	// photo mode
	bool rotateByMouse = false;

};
