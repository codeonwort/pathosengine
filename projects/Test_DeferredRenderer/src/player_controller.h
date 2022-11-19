#pragma once

#include "pathos/scene/actor.h"
using namespace pathos;

class PlayerController : public Actor
{
	
public:
	virtual void onSpawn() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void setupInput();

	bool rotateByMouse = false;

};
