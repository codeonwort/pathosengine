#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
using namespace pathos;

#include <vector>

class PlayerController;

class World_RC1 : public World {

public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupSky();
	void setupScene();

	void updateStarfield();

private:
	PlayerController* playerController = nullptr;
	
	GLuint starfield = 0;

};
