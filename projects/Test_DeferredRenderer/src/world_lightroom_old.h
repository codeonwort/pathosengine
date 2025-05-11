#pragma once

#include "pathos/scene/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

#include <vector>

// --------------------------------------------------------

namespace pathos {
	class StaticMeshActor;
	class PointLightActor;
	class DirectionalLightActor;
	class RectLightActor;
}

class PlayerController;

// --------------------------------------------------------

// A world to test lighting features.
class World_LightRoomOld : public World {

public:
	void onInitialize() override;
	void onTick(float deltaSeconds) override;

private:
	void setupInput();
	void setupScene();

private:
	actorPtr<StaticMeshActor> ground;
	actorPtr<StaticMeshActor> wallA;
	actorPtr<StaticMeshActor> wallB;
	actorPtr<StaticMeshActor> box;
	actorPtr<StaticMeshActor> ball;
	actorPtr<DirectionalLightActor> sun;
	actorPtr<PointLightActor> pointLight0;
	actorPtr<StaticMeshActor> pointLight0Gizmo;
	actorPtr<RectLightActor> rectLight0;
	actorPtr<StaticMeshActor> rectLight0Gizmo;
	actorPtr<PlayerController> playerController;
};
