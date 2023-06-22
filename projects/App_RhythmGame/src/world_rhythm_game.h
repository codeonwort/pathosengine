#pragma once

#include "pathos/scene/world.h"
using namespace pathos;

class World_RhythmGame : public World {

protected:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;
	virtual void onDestroy() override;

};
