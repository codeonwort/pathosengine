#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/scene/world.h"
#include "pathos/scene/actor.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

#include <vector>

namespace pathos {
	class SkyboxActor;
	class DirectionalLightActor;
	class StaticMeshComponent;
	class MeshGeometry;
	class Material;
	class StaticMesh;
}
class PlayerController;

// -----------------------------------------------------------------------
// SquareDiamondActor

class SquareDiamondActor : public Actor {

public:
	SquareDiamondActor();

	void buildMesh(float R, float D);

private:
	std::vector<MeshGeometry*> geometries;
	std::vector<assetPtr<Material>> materials;
	std::vector<assetPtr<StaticMesh>> staticMeshAssets;
	std::vector<StaticMeshComponent*> staticMeshComponents;
};

// -----------------------------------------------------------------------
// World_RC2

class World_RC2 : public World {

public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	PlayerController* playerController = nullptr;
	DirectionalLightActor* sunActor = nullptr;
	std::vector<SquareDiamondActor*> squareDiamonds;

	SkyboxActor* skyActor = nullptr;
	assetPtr<Material> skyMaterial;
};
