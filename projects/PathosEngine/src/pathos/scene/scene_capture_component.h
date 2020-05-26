#pragma once

#include "pathos/actor/scene_component.h"

#include "gl_core.h"
#include <vector>

namespace pathos {

	class SceneComponent;
	class RenderTarget2D;

	// #todo-scene-capture: Support various result format.
	//enum class SceneCaptureResult : uint8 {
	//	RawHDR,   // Unpacking of G-buffer, before tone-mapping.
	//	ToneMappedLDR,
	//	DepthOnly
	//};

	// #todo-scene-capture: Support a way to capture only specific actors
	//enum class SceneCaptureMethod : uint8 {
	//	CaptureEverything,
	//	CaptureOnlySpecified,
	//};

	// Captures the scene at its current location and rotation, and then write the result to a render target.
	// Basically it performs whole scene rendering again.
	class SceneCaptureComponent : public SceneComponent {

	public:
		SceneCaptureComponent() = default;
		virtual ~SceneCaptureComponent() = default;

		void captureScene();

		// #todo-scene-capture
		//void addCaptureTarget(Actor* targetActor);
		//void clearCaptureTargets();
		//SceneCaptureMethod captureMethod = SceneCaptureMethod::CaptureEverything;

	public:
		float fovY = 60.0f; // in degrees
		//float aspectRatio; // This is derived from the render target's (width/height)
		float zNear = 1.0f;
		float zFar = 10000.0f;
		RenderTarget2D* renderTarget = nullptr;

	private:
		std::vector<Actor*> actorsToCapture;

	};

}
