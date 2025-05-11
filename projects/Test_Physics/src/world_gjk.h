#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

#include "badger/physics/shape.h"

namespace pathos {
	class SkyAtmosphereActor;
	class DirectionalLightActor;
	class StaticMeshActor;
	class Material;
}
class PlayerController;

class World_GJK : public World {

public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	actorPtr<PlayerController> controller;

	actorPtr<StaticMeshActor> modelA;
	actorPtr<StaticMeshActor> modelB;
	actorPtr<StaticMeshActor> arrow;

	badger::physics::Body bodyA;
	badger::physics::Body bodyB;

	assetPtr<Material> materialNoHit;
	assetPtr<Material> materialOnHit;

};
