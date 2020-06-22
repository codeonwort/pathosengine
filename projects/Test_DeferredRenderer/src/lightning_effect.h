#pragma once

#include "pathos\actor\actor.h"
#include "pathos\mesh\static_mesh_component.h"
using namespace pathos;

namespace pathos {
	class StaticMeshComponent;
	class SphereGeometry;
	class ProceduralGeometry;
	class ColorMaterial;
}

class LightningParticleComponent : public StaticMeshComponent {
public:
	LightningParticleComponent();
	void generateParticle(const vector3& p0, const vector3& p1);
private:
	ProceduralGeometry* G;
	ColorMaterial* M;
};

class LightningActor : public Actor {
	
public:
	LightningActor();

	void generateParticle(const vector3& p0, const vector3& p1);
	
private:
	StaticMeshComponent* sphereComponent;
	SphereGeometry* sphereGeometry;
	ColorMaterial* sphereMaterial;

	LightningParticleComponent* particleComponent;
};
