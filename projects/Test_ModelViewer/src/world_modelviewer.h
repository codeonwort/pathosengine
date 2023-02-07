#pragma once

#include "pathos/scene/world.h"
using namespace pathos;

namespace pathos {
	class StaticMeshActor;
	class OBJLoader;
}

class World_ModelViewer : public World {

protected:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void registerConsoleCommands();
	void tryLoadModel(const char* filepath);
	void onLoadOBJ(OBJLoader* loader, uint64 payload);

private:
	StaticMeshActor* modelActor = nullptr;

};
