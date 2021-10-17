#pragma once

#include "pathos/actor/actor.h"
using namespace pathos;

class PlayerController : public Actor
{

public:
	virtual void onSpawn() override;
	virtual void onTick(float deltaSeconds) override;

	void setPlayerPawn(Actor* player);
	void togglePhotoMode();

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
