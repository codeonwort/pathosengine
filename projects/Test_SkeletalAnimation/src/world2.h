#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
using namespace pathos;

//////////////////////////////////////////////////////////////////////////

namespace pathos {
	class StaticMeshActor;
	class SkinnedMesh;
	class DirectionalLightActor;
	class PointLightActor;
	class TextMeshActor;
}
class PlayerController;

class World2 : public World {
	
public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupScene();
	void loadDAE();

private:
	PlayerController* playerController = nullptr;

	DirectionalLightActor* dirLight = nullptr;
	PointLightActor* pointLight0 = nullptr;

	StaticMeshActor* model = nullptr;
	StaticMeshActor* model2 = nullptr;
	SkinnedMesh* daeModel = nullptr;
	SkinnedMesh* daeModel2 = nullptr;
	std::vector<StaticMeshActor*> boxes;

	TextMeshActor* text1 = nullptr;

};
