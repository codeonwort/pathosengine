#pragma once

#include "pathos/actor/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

#include <vector>

// #todo: Actors are manually free'd in World::destroy() for now.
// Using sharedPtr causes double free = crash.
#define SHARED_PTR_ACTORS 0

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
#if SHARED_PTR_ACTORS
	sharedPtr<StaticMeshActor> ground;
	sharedPtr<StaticMeshActor> box;
	sharedPtr<DirectionalLightActor> sun;
	sharedPtr<PointLightActor> pointLight0;
	sharedPtr<StaticMeshActor> pointLight0Gizmo;
#else
	StaticMeshActor* ground = nullptr;
	StaticMeshActor* box = nullptr;
	DirectionalLightActor* sun = nullptr;
	PointLightActor* pointLight0 = nullptr;
	StaticMeshActor* pointLight0Gizmo = nullptr;
#endif
};
