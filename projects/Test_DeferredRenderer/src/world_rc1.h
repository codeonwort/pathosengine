#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

#include <vector>
#include "badger/math/spline.h"

namespace pathos {
	class ProceduralGeometry;
	class Texture;
	class Material;
	class OBJLoader;
	class TextMeshActor;
	class PanoramaSkyActor;
}
class PlayerController;
class LightningActor;

class RingActor : public StaticMeshActor {
public:
	RingActor();

	void buildRing(float innerRadius, float outerRadius, float thickness, const std::vector<float>& segmentRanges);
	vector3 getRandomInnerPosition() const;

private:
	assetPtr<ProceduralGeometry> G;
	assetPtr<Material> M;
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
	actorPtr<PlayerController> playerController;
	actorPtrList<RingActor> rings;

	actorPtr<LightningActor> lightningSphere;
	std::vector<uint32> ringIndicesForParticleRotation;

	actorPtr<SpaceshipActor> spaceship1;
	actorPtr<SpaceshipActor> spaceship2;
	actorPtr<StaticMeshActor> guardTower;

	assetPtr<Material> M_tower;
	
	actorPtr<PanoramaSkyActor> panoramaSky;
	Texture* starfieldTexture = nullptr;

};
