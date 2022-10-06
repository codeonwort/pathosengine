#pragma once

#include "pathos/actor/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

// --------------------------------------------------------

namespace pathos {
	class StaticMeshActor;
	class PointLightActor;
	class DirectionalLightActor;
	class RectLightActor;
	class GLTFLoader;
}

class PlayerController;

// --------------------------------------------------------

// Yet another Sponza scene.
class World_Sponza : public World {

public:
	void onInitialize() override;
	void onTick(float deltaSeconds) override;

private:
	void onLoadGLTF(GLTFLoader* loader, uint64 payload);

	void setupInput();
	void setupScene();

private:
	StaticMeshActor* ground = nullptr;
	DirectionalLightActor* sun = nullptr;

	PlayerController* playerController = nullptr;
};
