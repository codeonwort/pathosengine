#pragma once

#include "pathos/scene/camera.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/mesh/geometry_procedural.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

#include "badger/types/vector_types.h"

class CSMDebugger : public StaticMeshActor {

public:
	CSMDebugger();

	void drawCameraFrustum(const Camera& camera, const vector3& sunDirection);

private:
	assetPtr<ProceduralGeometry> G;
	assetPtr<ProceduralGeometry> G2;

};
