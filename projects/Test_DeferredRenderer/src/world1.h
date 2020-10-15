#pragma once

#include "pathos/engine.h"
#include "pathos/actor/world.h"
using namespace pathos;

#include <vector>

//////////////////////////////////////////////////////////////////////////

#define VISUALIZE_CSM_FRUSTUM 1
#define DEBUG_SKYBOX          0

#if VISUALIZE_CSM_FRUSTUM
#include "csm_debugger.h"
#endif

extern const vector3       CAMERA_POSITION;
extern const vector3       CAMERA_LOOK_AT;
extern const vector3       SUN_DIRECTION;
extern const vector3       SUN_RADIANCE;

// Forward declaration
namespace pathos {
	class OBJLoader;
	class StaticMeshActor;
	class SceneCaptureComponent;
}
class CSMDebugger;
class PlayerController;

// World1
class World1 : public World {

public:
	World1();

	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupInput();
	void setupSky();
	void setupScene();
	void setupCSMDebugger();
	void onLoadOBJ(OBJLoader* loader);

private:
	StaticMeshActor* objModel = nullptr;
	std::vector<StaticMeshActor*> balls;
	std::vector<StaticMeshActor*> boxes;
	StaticMeshActor* godRaySource = nullptr;
	StaticMeshActor* ground = nullptr;
	SceneCaptureComponent* sceneCaptureComponent = nullptr;
#if VISUALIZE_CSM_FRUSTUM
	CSMDebugger* csmDebugger = nullptr;
#endif
	PlayerController* playerController = nullptr;

};
