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
	class OBJLoader;
	class LightProbeActor;
}
class PlayerController;

class World2 : public World {
	
public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupScene();
	void loadDAE();

	void onLoadWavefrontOBJ(OBJLoader* loader, uint64 payload);

private:
	PlayerController* playerController = nullptr;

	DirectionalLightActor* sunLight = nullptr;
	PointLightActor* pointLight0 = nullptr;

	StaticMeshActor* ground = nullptr;
	StaticMeshActor* godRaySourceMesh = nullptr;

	StaticMeshActor* lpsHead = nullptr;

	SkinnedMesh* daeModel_my = nullptr;
	SkinnedMesh* daeModel_riggedFigure = nullptr;

	TextMeshActor* alertText1 = nullptr;
	TextMeshActor* alertText2 = nullptr;

	LightProbeActor* lightProbe0 = nullptr;

};
