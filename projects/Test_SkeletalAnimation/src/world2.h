#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
using namespace pathos;

//////////////////////////////////////////////////////////////////////////

namespace pathos {
	class StaticMeshActor;
	class SkinnedMesh;
}

class World2 : public World {
	
public:
	World2();

	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupInput();
	void setupScene();
	void loadDAE();

private:
	StaticMeshActor* model = nullptr;
	StaticMeshActor* model2 = nullptr;
	SkinnedMesh* daeModel = nullptr;
	SkinnedMesh* daeModel2 = nullptr;
	std::vector<StaticMeshActor*> boxes;

};
