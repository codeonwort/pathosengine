#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
using namespace pathos;

namespace pathos {
	class VolumetricCloudActor;
	class SkyAtmosphereActor;
	class SkyboxActor;
	class PanoramaSkyActor;
	class DirectionalLightActor;
	class PointLightActor;
	class StaticMeshActor;
	class Mesh;
	class Texture;
	class OBJLoader;
}
class PlayerController;

class World_Game1 : public World {
	
public:
	// BEGIN_INTERFACE: World
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;
	// END_INTERFACE: World

	void prepareAssets();
	void reloadScene();
	void setupScene();

private:
	void onLoadOBJ(OBJLoader* loader, uint64 payload);

private:
	GLuint weatherTexture = 0;
	Texture* cloudShapeNoise = nullptr;
	Texture* cloudErosionNoise = nullptr;
	VolumetricCloudActor* cloudscape = nullptr;

	SkyAtmosphereActor* skyAtmosphere = nullptr;
	SkyboxActor* skybox = nullptr;
	PanoramaSkyActor* skyEquimap = nullptr;

	DirectionalLightActor* sun = nullptr;
	PointLightActor* pointLight0 = nullptr;

	StaticMeshActor* playerCar = nullptr;
	StaticMeshActor* landscape = nullptr; // #todo-landscape: Implement LandscapeActor

	Mesh* carMesh = nullptr;
	Mesh* carDummyMesh = nullptr;
	Mesh* landscapeMesh = nullptr;

	std::vector<StaticMeshActor*> treeActors;
	Mesh* treeMesh = nullptr;

	PlayerController* playerController = nullptr;
};
