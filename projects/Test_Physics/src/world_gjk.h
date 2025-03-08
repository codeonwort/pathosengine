#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
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
	PlayerController* controller = nullptr;

	StaticMeshActor* modelA = nullptr;
	StaticMeshActor* modelB = nullptr;
	StaticMeshActor* arrow = nullptr;

	badger::physics::Body bodyA;
	badger::physics::Body bodyB;

	Material* materialNoHit = nullptr;
	Material* materialOnHit = nullptr;

};
