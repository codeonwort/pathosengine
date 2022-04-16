#pragma once

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"
#include "badger/system/stopwatch.h"

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <condition_variable>
#include "gl_core.h"

namespace pathos {

	class SceneProxy;
	class OpenGLDevice;
	class Renderer;
	class OverlayRenderer;
	class DebugOverlay;

	class RenderThread final : public Noncopyable {
		static void renderThreadMain(RenderThread* renderThread);

	public:
		RenderThread();
		~RenderThread();

		// Launches the render thread.
		void run();

		void beginFrame(uint32 frameNumber);
		void endFrame(uint32 frameNumber);

		void terminate();

		inline uint32 getThreadID() const { return threadID; }
		inline const std::string& getThreadName() const { return threadName; }

		inline OverlayRenderer* getRenderer2D() const { return renderer2D; }

		//
		// sceneProxyQueue helper methods.
		//
		bool isSceneProxyQueueEmpty();
		SceneProxy* popSceneProxy();
		void pushSceneProxy(SceneProxy* inSceneProxy);

	// #todo-renderthread-fatal: Called by the main thread due to initialization order.
	private:
		bool                       initializeOpenGL();
		bool                       initializeOverlayRenderer();
		bool                       initializeRenderer(RenderCommandList& cmdList);

		bool                       destroyOpenGL();
	public:
		inline void                markMainLoopStarted() { mainLoopStarted = true; }
		void                       waitForInitialization();

	// Render thread
	private:
		uint32                     threadID;
		std::string                threadName;
		std::thread                nativeThread;
		std::mutex                 loopMutex;
		std::condition_variable    loopCondVar;
		std::atomic<bool>          bPendingKill;

		std::atomic<bool>          mainLoopStarted;

		std::atomic<bool>          bInitialized;
		std::mutex                 initMutex;
		std::condition_variable    initCondVar;

		OpenGLDevice*              render_device;
		Renderer*                  renderer;
		OverlayRenderer*           renderer2D;
		DebugOverlay*              debugOverlay;

		uint32                     currentFrame;
		Stopwatch                  stopwatch; // for render thread
		float                      elapsed_renderThread;

		std::queue<SceneProxy*>    sceneProxyQueue; // CAUTION: No direct access!!! Use helper methods.
		std::mutex                 sceneProxyQueueMutex;

	// GPU
	private:
		GLuint                     gpuTimerQuery;
		float                      elapsed_gpu; // in milliseconds
		std::vector<std::string>   lastGpuCounterNames;
		std::vector<float>         lastGpuCounterTimes;
	};

}
