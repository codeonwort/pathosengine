#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
using namespace pathos;

class World_GJK : public World {

public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	//

};
