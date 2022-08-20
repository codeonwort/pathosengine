#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
#include "pathos/mesh/static_mesh_actor.h"
using namespace pathos;

#include <vector>
#include "badger/math/spline.h"

namespace pathos {
	class ProceduralGeometry;
	class ColorMaterial;
	class OBJLoader;
	class TextMeshActor;
}
class PlayerController;
class LightningActor;

class RingActor : public StaticMeshActor {
public:
	RingActor();
	void buildRing(float innerRadius, float outerRadius, float thickness, const std::vector<float>& segmentRanges);
	vector3 getRandomInnerPosition() const;
private:
	class ProceduralGeometry* G;
	class ColorMaterial* M;
	std::vector<uint32> innerVertexIndices; // For lightning effect pivot
};

class SpaceshipActor : public StaticMeshActor {
public:
	virtual void onSpawn() override;
	virtual void onDestroy() override {}
	virtual void onTick(float deltaSeconds) override {}
	HermiteSpline& getSpline() { return spline; }
	void setSpline(const HermiteSpline& inSpline) { spline = inSpline; }
private:
	void onLoadOBJ(OBJLoader* loader, uint64 payload);

	HermiteSpline spline;
};

class World_RC1 : public World {

public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupSky();
	void setupScene();

	void updateStarfield();

	void onLoadOBJ(OBJLoader* loader, uint64 payload);

private:
	PlayerController* playerController = nullptr;
	std::vector<RingActor*> rings;

	LightningActor* lightningSphere = nullptr;
	std::vector<uint32> ringIndicesForParticleRotation;

	SpaceshipActor* spaceship1 = nullptr;
	SpaceshipActor* spaceship2 = nullptr;
	StaticMeshActor* guardTower = nullptr;
	
	GLuint starfield = 0;

};
