#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
using namespace pathos;

extern const vector3       CAMERA_POSITION;
extern const vector3       CAMERA_LOOK_AT;

namespace pathos {
	class DirectionalLightActor;
	class PointLightActor;
	class StaticMeshActor;
}

class World_Game1 : public World {
	
public:
	World_Game1();

	// BEGIN_INTERFACE: World
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;
	// END_INTERFACE: World

	void setupScene();

private:
	DirectionalLightActor* sun;
	PointLightActor* pointLight0;
	StaticMeshActor* sphere0;

};
