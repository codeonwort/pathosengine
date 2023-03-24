#pragma once

#include "pathos/scene/actor.h"
#include "pathos/scene/static_mesh_component.h"
using namespace pathos;

namespace pathos {
	class StaticMeshComponent;
	class SphereGeometry;
	class ProceduralGeometry;
}

class LightningParticleComponent : public StaticMeshComponent {
public:
	LightningParticleComponent();

	void setParameters(GLuint maskTexture, GLuint warpTexture, float rc1Scale);
	void generateParticle(const vector3& p0, const vector3& p1);

private:
	ProceduralGeometry* G;
	Material* M;
};

class LightningActor : public Actor {
	
public:
	LightningActor();

	std::vector<LightningParticleComponent*>& getParticleComponents() { return particleComponents; }
	void generateParticle(const vector3& p0, const vector3& p1, float rc1Scale);

	virtual void onSpawn() override;
	virtual void onDestroy() override;
	
private:
	StaticMeshComponent* sphereComponent = nullptr;
	std::vector<LightningParticleComponent*> particleComponents;
	GLuint maskTexture = 0;
	GLuint warpTexture = 0;
};
