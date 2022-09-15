#pragma once

#include "pathos/actor/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

#include <vector>

// --------------------------------------------------------

namespace pathos {
	class StaticMeshActor;
	class PointLightActor;
	class DirectionalLightActor;
}

// --------------------------------------------------------

// A world to test lighting features.
class World_LightRoom : public World {

public:
	void onInitialize() override;
	void onTick(float deltaSeconds) override;

private:
	void setupScene();

private:
	sharedPtr<StaticMeshActor> ground;
	sharedPtr<StaticMeshActor> box;

	sharedPtr<DirectionalLightActor> sun;

	sharedPtr<PointLightActor> pointLight0;
	sharedPtr<StaticMeshActor> pointLight0Gizmo;
};
