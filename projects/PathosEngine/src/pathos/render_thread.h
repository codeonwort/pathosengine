#pragma once

#include "pathos/rhi/gl_handles.h"

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"
#include "badger/system/stopwatch.h"
#include "badger/system/cpu.h"

#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>

namespace pathos {

	class SceneProxy;
	class OpenGLDevice;
	class Renderer;
	class OverlayRenderer;
	class DebugOverlay;
	class OverlaySceneProxy;
	struct SceneRenderTargets;

	class RenderThread final : public Noncopyable {
		static void renderThreadMain(RenderThread* renderThread);

	public:
		RenderThread();
		~RenderThread();

		// Launches the render thread.
		void run();

		// #todo-renderthread: I forgot why I declared this
		// Signal fence when GPU has completed commands.
		//void signalFence(uint64 value);

		void takeScreenshot() { bScreenshotReserved = true; }

		// NOTE: Blocking operation.
		void terminate();

		inline PlatformThreadId getThreadID() const { return threadID; }
		inline const std::string& getThreadName() const { return threadName; }

		inline OverlayRenderer* getRenderer2D() const { return renderer2D; }
		inline DebugOverlay* getDebugOverlay() const { return debugOverlay; }

		//
		// sceneProxyQueue helper methods.
		//
		bool isSceneProxyQueueEmpty();
		bool mainSceneInSceneProxyQueue();
		SceneProxy* popSceneProxy();
		void pushSceneProxy(SceneProxy* inSceneProxy);

		//
		// overlayProxyQueue helpers.
		//
		bool isOverlayProxyQueueEmpty();
		OverlaySceneProxy* popOverlayProxy();
		void pushOverlayProxy(OverlaySceneProxy* inOverlayProxy);

	// #note-renderthread: Called by the main thread due to initialization order.
	private:
		bool                              initializeOpenGL();
		bool                              initializeOverlayRenderer();
		bool                              initializeRenderer(RenderCommandList& cmdList);

		bool                              destroyOpenGL();
	public:
		inline void                       markMainLoopStarted() { mainLoopStarted = true; }
		void                              waitForInitialization();

	// Render thread
	private:
		PlatformThreadId                  threadID;
		std::string                       threadName;
		std::thread                       nativeThread;
		std::mutex                        loopMutex;
		std::condition_variable           loopCondVar;

		std::atomic<bool>                 bPendingKill;
		std::mutex                        terminateMutex;
		std::condition_variable           terminateCondVar;

		std::atomic<bool>                 mainLoopStarted;

		std::atomic<bool>                 bInitialized;
		std::mutex                        initMutex;
		std::condition_variable           initCondVar;

		OpenGLDevice*                     renderDevice = nullptr;

		Renderer*                         renderer = nullptr;
		SceneRenderTargets*               sceneRenderTargets_primary = nullptr;         // SceneProxySource::MainScene, SceneCapture
		SceneRenderTargets*               sceneRenderTargets_radianceProbe = nullptr;   // SceneProxySource::RadianceCapture
		SceneRenderTargets*               sceneRenderTargets_irradianceProbe = nullptr; // SceneProxySource::IrradianceCapture

		OverlayRenderer*                  renderer2D = nullptr;
		DebugOverlay*                     debugOverlay = nullptr;

		Stopwatch                         stopwatch; // for render thread
		float                             elapsed_renderThread = 0.0f;

		std::list<SceneProxy*>            sceneProxyQueue; // CAUTION: No direct access!!! Use helper methods.
		std::mutex                        sceneProxyQueueMutex;

		std::list<OverlaySceneProxy*>     overlayProxyQueue; // CAUTION: No direct access!!! Use helper methods.
		std::mutex                        overlayProxyQueueMutex;

		bool                              bScreenshotReserved = false;

	// GPU
	private:
		GLuint                            gpuTimerQuery = 0;
		float                             elapsed_gpu = 0.0f; // in milliseconds
		std::vector<std::string>          lastGpuCounterNames;
		std::vector<float>                lastGpuCounterTimes;
	};

}
