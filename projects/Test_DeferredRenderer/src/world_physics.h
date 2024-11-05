#pragma once

#include "pathos/scene/world.h"
using namespace pathos;

class World_Physics : public World {

public:
	void onInitialize() override;
	void onTick(float deltaSeconds) override;

private:
	//

};
