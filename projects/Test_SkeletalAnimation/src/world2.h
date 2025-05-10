#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

//////////////////////////////////////////////////////////////////////////

namespace pathos {
	class StaticMeshActor;
	class SkinnedMesh;
	class DirectionalLightActor;
	class PointLightActor;
	class TextMeshActor;
	class OBJLoader;
	class ReflectionProbeActor;
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
	actorPtr<PlayerController> playerController;
	actorPtr<DirectionalLightActor> sunLight;
	actorPtr<PointLightActor> pointLight0;
	actorPtr<StaticMeshActor> ground;
	actorPtr<StaticMeshActor> godRaySourceMesh;
	actorPtr<StaticMeshActor> lpsHead;

	assetPtr<SkinnedMesh> daeModel_my;
	assetPtr<SkinnedMesh> daeModel_riggedFigure;

	actorPtr<TextMeshActor> alertText1;
	actorPtr<TextMeshActor> alertText2;

};
