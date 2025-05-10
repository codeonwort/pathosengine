#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

namespace pathos {
	class VolumetricCloudActor;
	class SkyAtmosphereActor;
	class SkyboxActor;
	class PanoramaSkyActor;
	class DirectionalLightActor;
	class PointLightActor;
	class StaticMeshActor;
	class LandscapeActor;
	class StaticMesh;
	class Material;
	class Texture;
	class OBJLoader;
}
class PlayerController;

class World_RacingGame : public World {
	
public:
	// BEGIN_INTERFACE: World
	virtual void onInitialize() override;
	virtual void onDestroy() override;
	virtual void onTick(float deltaSeconds) override;
	// END_INTERFACE: World

	void prepareAssets();
	void reloadScene();
	void setupScene();

private:
	void onLoadOBJ(OBJLoader* loader, uint64 payload);

private:
	Texture* weatherTexture = nullptr;
	Texture* cloudShapeNoise = nullptr;
	Texture* cloudErosionNoise = nullptr;
	actorPtr<VolumetricCloudActor> cloudscape;

	actorPtr<SkyAtmosphereActor> skyAtmosphere;
	actorPtr<SkyboxActor> skybox;
	actorPtr<PanoramaSkyActor> skyEquimap;
	actorPtr<DirectionalLightActor> sun;
	actorPtr<PointLightActor> pointLight0;
	actorPtr<StaticMeshActor> playerCar;
	actorPtr<LandscapeActor> landscape;

	actorPtr<StaticMesh> carMesh;
	actorPtr<StaticMesh> carDummyMesh;

	actorPtrList<StaticMeshActor> treeActors;
	actorPtr<StaticMesh> treeMesh;

	actorPtr<PlayerController> playerController;
};
