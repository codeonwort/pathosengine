#pragma once

#include "pathos/actor/actor_component.h"

#include "gl_core.h"
#include <vector>

// #todo-scene-capture: Move to a separate file.
namespace pathos {

	enum class RenderTargetFormat : uint8
	{
		RGBA16F,
		RGBA16I,
		RGBA16UI,

		RGBA32F,
		RGBA32I,
		RGBA32UI,

		DEPTH16F,
		DEPTH24F,
		DEPTH32F,
	};

	// Wrapper for a 2D texture which can be used as a render target.
	class RenderTarget2D final {

	public:
		RenderTarget2D() = default;
		~RenderTarget2D();

		void initializeResource();
		void destroyResource();

		void respecTexture(uint32 inWidth, uint32 inHeight, RenderTargetFormat inFormat);

		inline uint32 getWidth() const { return width; }
		inline uint32 getHeight() const { return height; }

		inline GLuint getGLName() const { return glTextureObject; }
		
	private:
		// Returns true if GL texture is invalid or its current spec doesn't match with the render target spec.
		bool isTextureInvalid() const;

		GLuint glTextureObject = 0;
		uint32 width;
		uint32 height;
		RenderTargetFormat format;

	};

}

namespace pathos {

	class SceneComponent;

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
	class SceneCaptureComponent : public ActorComponent {

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

	private:
		std::vector<Actor*> actorsToCapture;
		RenderTarget2D* renderTarget = nullptr;

	};

}
