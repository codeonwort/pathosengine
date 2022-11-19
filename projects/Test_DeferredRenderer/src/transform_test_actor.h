#pragma once

#include "pathos/scene/actor.h"
using namespace pathos;

#include <vector>

namespace pathos {
	class StaticMeshComponent;
}

class TransformTestActor : public Actor {

public:
	TransformTestActor();

	void onTick(float deltaSeconds) override;

private:
	StaticMeshComponent* root;
	std::vector<StaticMeshComponent*> stars;
	std::vector<StaticMeshComponent*> moons;

};
