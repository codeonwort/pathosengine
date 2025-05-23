#pragma once

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/smart_pointer.h"
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
	actorPtrList<StaticMeshActor> balls;
	actorPtrList<StaticMeshActor> pillars;
	actorPtr<StaticMeshActor> godRaySource;
	actorPtr<StaticMeshActor> ground;
#if VISUALIZE_CSM_FRUSTUM
	actorPtr<CSMDebugger> csmDebugger;
#endif
	actorPtr<TransformTestActor> transformTestActor;
	actorPtr<PlayerController> playerController;

	// For scene capture
	RenderTarget2D* tempRenderTarget = nullptr;
	actorPtr<Actor> sceneCaptureActor;
	SceneCaptureComponent* sceneCaptureComponent = nullptr;
};
