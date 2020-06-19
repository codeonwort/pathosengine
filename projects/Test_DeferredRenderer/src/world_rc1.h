#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
#include "pathos/mesh/static_mesh_actor.h"
using namespace pathos;

#include <vector>

namespace pathos {
	class ProceduralGeometry;
	class ColorMaterial;
}
class PlayerController;

class RingActor : public StaticMeshActor {
public:
	RingActor();
	void buildRing(float innerRadius, float outerRadius);
private:
	class ProceduralGeometry* G;
	class ColorMaterial* M;
};

class World_RC1 : public World {

public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupSky();
	void setupScene();

	void updateStarfield();

private:
	PlayerController* playerController = nullptr;
	StaticMeshActor* sphere = nullptr;
	std::vector<RingActor*> rings;
	
	GLuint starfield = 0;

};
