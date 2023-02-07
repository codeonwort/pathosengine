#pragma once

#include "pathos/scene/world.h"
using namespace pathos;

namespace pathos {
	class DirectionalLightActor;
	class StaticMeshActor;
	class OBJLoader;
}
class PlayerController;

class World_ModelViewer : public World {

protected:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void registerConsoleCommands();
	void tryLoadModel(const char* filepath);
	void onLoadOBJ(OBJLoader* loader, uint64 payload);

private:
	DirectionalLightActor* sun = nullptr;
	PlayerController* playerController = nullptr;
	StaticMeshActor* modelActor = nullptr;
	StaticMeshActor* dummyBox = nullptr;

	vector3 sunDirection = vector3(0.0f, -1.0f, -1.0f);
	vector3 sunIlluminance = vector3(10.0f);

};
