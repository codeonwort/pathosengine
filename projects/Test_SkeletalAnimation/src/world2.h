#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
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

	DirectionalLightActor* sunLight = nullptr;
	PointLightActor* pointLight0 = nullptr;

	StaticMeshActor* ground = nullptr;
	StaticMeshActor* godRaySourceMesh = nullptr;

	SkinnedMesh* daeModel_my = nullptr;
	SkinnedMesh* daeModel_riggedFigure = nullptr;

	TextMeshActor* alertText1 = nullptr;
	TextMeshActor* alertText2 = nullptr;

};
