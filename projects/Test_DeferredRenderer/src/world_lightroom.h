#pragma once

#include "pathos/scene/world.h"
using namespace pathos;

namespace pathos {
	class GLTFLoader;
	class StaticMeshComponent;
}
class PlayerController;

class World_LightRoom : public World {

public:
	void onInitialize() override;
	void onTick(float deltaSeconds) override;

private:
	void onLoadGLTF(GLTFLoader* loader, uint64 payload);

	PlayerController* playerController = nullptr;
	StaticMeshComponent* ballComponent = nullptr;
};
