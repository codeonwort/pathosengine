#pragma once

#include "pathos/actor/scene_component.h"

#include "gl_core.h"
#include <vector>

namespace pathos {

	class SceneComponent;
	class RenderTarget2D;

	// Captures the scene at its current location and rotation, and then write the result to a render target.
	// Basically it performs whole scene rendering again.
	class SceneCaptureComponent : public SceneComponent {

	public:
		SceneCaptureComponent() = default;
		virtual ~SceneCaptureComponent() = default;

		void captureScene();

	public:
		float fovY = 60.0f; // in degrees
		float zNear = 1.0f;
		float zFar = 10000.0f;
		RenderTarget2D* renderTarget = nullptr;
		bool captureHDR = true; // Ignored if render target has depth format.

	private:
		std::vector<Actor*> actorsToCapture;
		uint32 sceneCaptureFrameNumber = 0;

	};

}
