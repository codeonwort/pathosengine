#pragma once

#include "pathos/scene/world.h"
using namespace pathos;

namespace pathos {
	class GLTFLoader;
	class StaticMeshComponent;
	class IrradianceVolumeActor;
}
class PlayerController;

class World_LightRoom : public World {

public:
	void onInitialize() override;
	void onTick(float deltaSeconds) override;

private:
	void onLoadGLTF(GLTFLoader* loader, uint64 payload);

	actorPtr<PlayerController> playerController;
	StaticMeshComponent* ballComponent = nullptr;

	std::vector<StaticMeshComponent*> fractures;
	std::vector<vector3> fractureOrigins;
	std::vector<vector3> fractureTargets;

	std::vector<StaticMeshComponent*> leafComponents;
	std::vector<vector3> leafOrigins;
	std::vector<vector3> leafTargets;

	actorPtrList<IrradianceVolumeActor> irradianceVolumes;
};
