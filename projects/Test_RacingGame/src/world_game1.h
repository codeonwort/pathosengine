#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
using namespace pathos;

extern const vector3       CAMERA_POSITION;
extern const vector3       CAMERA_LOOK_AT;

namespace pathos {
	class AtmosphereScattering;
	class Skybox;
	class AnselSkyRendering;
	class DirectionalLightActor;
	class PointLightActor;
	class StaticMeshActor;
	class Mesh;
}
class PlayerController;

class World_Game1 : public World {
	
public:
	World_Game1();

	// BEGIN_INTERFACE: World
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;
	// END_INTERFACE: World

	void prepareAssets();
	void reloadScene();
	void setupScene();

private:
	AtmosphereScattering* skyAtmosphere = nullptr;
	Skybox* skybox = nullptr;
	AnselSkyRendering* skyEquimap = nullptr;

	DirectionalLightActor* sun = nullptr;
	PointLightActor* pointLight0 = nullptr;
	StaticMeshActor* sphere0 = nullptr;
	StaticMeshActor* landscape = nullptr; // #todo-landscape: Implement LandscapeActor

	Mesh* sphereMesh = nullptr;
	Mesh* landscapeMesh = nullptr;

	PlayerController* playerController = nullptr;
};
