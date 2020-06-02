#pragma once

#include "badger/types/vector_types.h"
#include "pathos/camera/camera.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/mesh/geometry_procedural.h"
using namespace pathos;

class CSMDebugger : public StaticMeshActor {

public:
	CSMDebugger();

	void drawCameraFrustum(const Camera& camera, const vector3& sunDirection);

private:
	ProceduralGeometry* G = nullptr;
	ProceduralGeometry* G2 = nullptr;

};
