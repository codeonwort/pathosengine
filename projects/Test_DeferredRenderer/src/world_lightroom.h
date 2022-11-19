#pragma once

#include "pathos/scene/world.h"
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
	class RectLightActor;
}

class PlayerController;

// --------------------------------------------------------

// A world to test lighting features.
class World_LightRoom : public World {

public:
	void onInitialize() override;
	void onTick(float deltaSeconds) override;

private:
	void setupInput();
	void setupScene();

private:
#if SHARED_PTR_ACTORS
	sharedPtr<StaticMeshActor> ground;
	sharedPtr<StaticMeshActor> wallA;
	sharedPtr<StaticMeshActor> wallB;
	sharedPtr<StaticMeshActor> box;
	sharedPtr<StaticMeshActor> ball;
	sharedPtr<DirectionalLightActor> sun;
	sharedPtr<PointLightActor> pointLight0;
	sharedPtr<StaticMeshActor> pointLight0Gizmo;
	sharedPtr<RectLightActor> rectLight0;
	sharedPtr<StaticMeshActor> rectLight0Gizmo;

	sharedPtr<PlayerController> playerController;
#else
	StaticMeshActor* ground = nullptr;
	StaticMeshActor* wallA = nullptr;
	StaticMeshActor* wallB = nullptr;
	StaticMeshActor* box = nullptr;
	StaticMeshActor* ball = nullptr;
	DirectionalLightActor* sun = nullptr;
	PointLightActor* pointLight0 = nullptr;
	StaticMeshActor* pointLight0Gizmo = nullptr;
	RectLightActor* rectLight0 = nullptr;
	StaticMeshActor* rectLight0Gizmo = nullptr;

	PlayerController* playerController = nullptr;
#endif
};
