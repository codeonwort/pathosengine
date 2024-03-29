#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
using namespace pathos;

#include <vector>

// --------------------------------------------------------

#define VISUALIZE_CSM_FRUSTUM 1
#define DEBUG_SKYBOX          0

#if VISUALIZE_CSM_FRUSTUM
#include "csm_debugger.h"
#endif

// Forward declaration
namespace pathos {
	class OBJLoader;
	class StaticMeshActor;
	class SceneCaptureComponent;
	class RenderTarget2D;
}
class CSMDebugger;
class TransformTestActor;
class PlayerController;

// World1
class World1 : public World {

public:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

	void setupInput();
	void setupSky();
	void setupScene();
	void setupCSMDebugger();

private:
	std::vector<StaticMeshActor*> balls;
	std::vector<StaticMeshActor*> pillars;
	StaticMeshActor* godRaySource = nullptr;
	StaticMeshActor* ground = nullptr;
#if VISUALIZE_CSM_FRUSTUM
	CSMDebugger* csmDebugger = nullptr;
#endif
	TransformTestActor* transformTestActor = nullptr;
	PlayerController* playerController = nullptr;

	// For scene capture
	RenderTarget2D* tempRenderTarget = nullptr;
	Actor* sceneCaptureActor = nullptr;
	SceneCaptureComponent* sceneCaptureComponent = nullptr;
};
