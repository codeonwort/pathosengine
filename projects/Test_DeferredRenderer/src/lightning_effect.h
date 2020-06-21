#pragma once

#include "pathos\actor\actor.h"
using namespace pathos;

namespace pathos {
	class StaticMeshComponent;
	class SphereGeometry;
	class ColorMaterial;
}

class LightningActor : public Actor {
	
public:
	LightningActor();
	
private:
	StaticMeshComponent* sphereComponent;
	SphereGeometry* sphereGeometry;
	ColorMaterial* sphereMaterial;

};
