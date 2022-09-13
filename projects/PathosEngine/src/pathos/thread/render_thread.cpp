#include "render_thread.h"

#include "badger/thread/cpu.h"
#include "badger/assertion/assertion.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/debug_overlay.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/text/font_mgr.h"
#include "pathos/scene/scene_render_settings.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/renderer.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/render_deferred.h"
#include "pathos/util/log.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/gl_context_manager.h"
#include "pathos/overlay/display_object_proxy.h"

namespace pathos {

	void RenderThread::renderThreadMain(RenderThread* renderThread) {
		renderThread->threadID = CPU::getCurrentThreadId();
		pathos::gRenderThreadId = renderThread->threadID;
		CPU::setCurrentThreadName(L"Render Thread");

		CpuProfiler& cpuProfiler = CpuProfiler::getInstance();
		cpuProfiler.registerCurrentThread(renderThread->getThreadName().c_str());

		// Initialize
		{
			SCOPED_TAKE_GL_CONTEXT();

			renderThread->initializeOpenGL();

			RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();

			// Initialize subsystems
			gEngine->initializeFontSystem(cmdList);
			renderThread->initializeOverlayRenderer();
			gEngine->initializeConsole();
			renderThread->initializeRenderer(cmdList);

			// Invoke global init routines
			auto& initRoutines = gEngine->getGlobalRenderRoutineContainer().initRoutines;
			for (Engine::GlobalRenderRoutine routine : initRoutines) {
				routine(gRenderDevice, cmdList);
			}

			// Notify end of initialization
			glFinish(); // glFinish to empty GPU works and safely detach the GL context.
			renderThread->bInitialized = true;
			renderThread->initCondVar.notify_all();
		}

		// Main loop for render thread
		while (renderThread->bPendingKill == false) {
			//
			// Start a frame!
			//
			OpenGLContextManager::takeContext();
			bool bNewSceneRendered = false;

			SceneProxy* sceneProxy = renderThread->popSceneProxy();
			bool bMainScene = (sceneProxy == nullptr) || (sceneProxy->sceneProxySource == SceneProxySource::MainScene);

			Renderer* renderer = renderThread->renderer;
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();
			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();

			GLuint64 gpu_elapsed_ns;
			immediateContext.beginQuery(GL_TIME_ELAPSED, renderThread->gpuTimerQuery);

			renderThread->stopwatch.start();

			SCOPED_CPU_COUNTER(EngineRender);

			const EngineConfig engineConfig(gEngine->getConfig());
			const int32 screenWidth = engineConfig.windowWidth;
			const int32 screenHeight = engineConfig.windowHeight;
			if (renderer) {
				if (sceneProxy && sceneProxy->bSceneRenderSettingsOverriden) {
					// Probably scene capture
					renderer->setSceneRenderSettings(sceneProxy->sceneRenderSettingsOverride);
				} else {
					// Main scene
					SceneRenderSettings settings;
					settings.sceneWidth = screenWidth; // #todo-renderer: Upscaling
					settings.sceneHeight = screenHeight;
					settings.frameCounter = renderThread->currentFrame;
					settings.enablePostProcess = true;
					renderer->setSceneRenderSettings(settings);
					renderer->setFinalRenderTargetToBackbuffer();
				}
			}
			
			deferredContext.flushAllCommands();

			// Renderer will add more immediate commands
			if (renderer && sceneProxy)
			{
				SCOPED_CPU_COUNTER(ExecuteRenderer);
				renderer->render(immediateContext, sceneProxy, &sceneProxy->camera);
				immediateContext.flushAllCommands();
				//deferredContext.flushAllCommands();

				// Update backbuffer only if main scene was actually rendered
				if (sceneProxy->sceneProxySource == SceneProxySource::MainScene) {
					bNewSceneRendered = true;
				}
			}

			// Render debug overlay and command console
			OverlaySceneProxy* overlayProxy = renderThread->popOverlayProxy();
			if (bMainScene && overlayProxy != nullptr) {
				if (overlayProxy->debugOverlayRootProxy != nullptr) {
					SCOPED_CPU_COUNTER(ExecuteDebugOverlay);
					renderThread->debugOverlay->renderDebugOverlay(
						immediateContext,
						overlayProxy->debugOverlayRootProxy,
						screenWidth,
						screenHeight);
					immediateContext.flushAllCommands();
				}
				if (gConsole && overlayProxy->consoleWindowRootProxy != nullptr) {
					SCOPED_CPU_COUNTER(ExecuteDebugConsole);
					gConsole->renderConsoleWindow(immediateContext, overlayProxy->consoleWindowRootProxy);
					immediateContext.flushAllCommands();
				}
			}

			immediateContext.endQuery(GL_TIME_ELAPSED);
			immediateContext.getQueryObjectui64v(renderThread->gpuTimerQuery, GL_QUERY_RESULT, &gpu_elapsed_ns);

			immediateContext.flushAllCommands();
			deferredContext.flushAllCommands();
			renderThread->elapsed_gpu = (float)gpu_elapsed_ns / 1000000.0f;

			// Get GPU profile
			const uint32 numGpuCounters = ScopedGpuCounter::flushQueries(
				&immediateContext,
				renderThread->lastGpuCounterNames,
				renderThread->lastGpuCounterTimes);

			// Clear render resources for current frame.
			if (sceneProxy != nullptr) {
				delete sceneProxy;
				sceneProxy = nullptr;
			}
			if (overlayProxy != nullptr) {
				delete overlayProxy;
				overlayProxy = nullptr;
			}

			// Let subsystems handle the end of rendering.
			FontManager::get().onFrameEnd();

			// Pass render stats to the game thread.
			renderThread->elapsed_renderThread = renderThread->stopwatch.stop() * 1000.0f;
			if (bNewSceneRendered) {
				gEngine->updateGPUQuery_renderThread(
					renderThread->elapsed_renderThread,
					renderThread->elapsed_gpu,
					renderThread->lastGpuCounterNames,
					renderThread->lastGpuCounterTimes);
			}

			//
			// End a frame
			//
			glFinish();
			OpenGLContextManager::returnContext();
			// Wait here and let GUI to take GL context and swap buffers.
			if (bNewSceneRendered) {
				gEngine->updateMainWindow_renderThread();
			}
		}

		// Cleanup thread main
		{
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();
			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();
			CHECKF(immediateContext.isEmpty(), "Immediate command list is not empty");
			CHECKF(deferredContext.isEmpty(), "Deferred command list is not empty");
		}

		LOG(LogInfo, "[%s] Render thread terminated", __FUNCTION__);
	}

	//////////////////////////////////////////////////////////

	RenderThread::RenderThread()
		: nativeThread()
		, threadID(0xffffffff)
		, threadName("Render Thread")
		, render_device(nullptr)
		, renderer(nullptr)
		, renderer2D(nullptr)
		, debugOverlay(nullptr)
		, currentFrame(0)
		, elapsed_renderThread(0.0f)
		, gpuTimerQuery(0)
		, elapsed_gpu(0.0f)
	{
		// Thread id and name are set in the thread entry point (renderThreadMain()).
	}

	RenderThread::~RenderThread() {
	}

	void RenderThread::run() {
		nativeThread = std::thread(renderThreadMain, this);
	}

	// #todo-renderthread: Was used for frame sync, but does nothing now.
	void RenderThread::beginFrame(uint32 frameNumber) {
		currentFrame = frameNumber;
	}

	// #todo-renderthread: Was used for frame sync, but does nothing now.
	void RenderThread::endFrame(uint32 frameNumber) {
		CHECKF(currentFrame == frameNumber, "Frame number does not match!!!");
		if (!bPendingKill) {
			//
		}
	}

	void RenderThread::terminate() {
		// #todo-renderthread: Should end RT loop first, then destroy RT resources.
		for (Engine::GlobalRenderRoutine routine : gEngine->getGlobalRenderRoutineContainer().destroyRoutines) {
			routine(gRenderDevice, gRenderDevice->getDeferredCommandList());
		}

		CHECK(destroyOpenGL());

		// #todo-renderthread: Does this ensure the lock will break?
		bPendingKill = true;
		loopCondVar.notify_all();
	}
	
	bool RenderThread::isSceneProxyQueueEmpty() {
		std::lock_guard<std::mutex> guard(sceneProxyQueueMutex);
		return sceneProxyQueue.size() == 0;
	}

	bool RenderThread::mainSceneInSceneProxyQueue() {
		std::lock_guard<std::mutex> guard(sceneProxyQueueMutex);
		bool bContains = false;
		for (SceneProxy* proxy : sceneProxyQueue) {
			if (proxy->sceneProxySource == SceneProxySource::MainScene) {
				bContains = true;
				break;
			}
		}
		return bContains;
	}

	SceneProxy* RenderThread::popSceneProxy() {
		std::lock_guard<std::mutex> guard(sceneProxyQueueMutex);
		if (sceneProxyQueue.empty()) {
			return nullptr;
		}
		SceneProxy* sceneProxy = sceneProxyQueue.front();
		sceneProxyQueue.pop_front();
		return sceneProxy;
	}

	void RenderThread::pushSceneProxy(SceneProxy* inSceneProxy) {
		std::lock_guard<std::mutex> guard(sceneProxyQueueMutex);
		sceneProxyQueue.push_back(inSceneProxy);
	}

	bool RenderThread::isOverlayProxyQueueEmpty() {
		std::lock_guard<std::mutex> guard(overlayProxyQueueMutex);
		return overlayProxyQueue.size() == 0;
	}

	OverlaySceneProxy* RenderThread::popOverlayProxy() {
		std::lock_guard<std::mutex> guard(overlayProxyQueueMutex);
		if (overlayProxyQueue.empty()) {
			return nullptr;
		}
		OverlaySceneProxy* overlayProxy = overlayProxyQueue.front();
		overlayProxyQueue.pop_front();
		return overlayProxy;
	}

	void RenderThread::pushOverlayProxy(OverlaySceneProxy* inOverlayProxy) {
		std::lock_guard<std::mutex> guard(overlayProxyQueueMutex);
		overlayProxyQueue.push_back(inOverlayProxy);
	}

	bool RenderThread::initializeOpenGL() {
		render_device = new OpenGLDevice;
		bool validDevice = render_device->initialize();
		
		if (!validDevice) {
			return false;
		}

		RenderCommandList& cmdList = render_device->getImmediateCommandList();

		render_device->genQueries(1, &gpuTimerQuery);
		CHECK(gpuTimerQuery != 0);

		// Create engine resources
		GLuint systemTextures[4];
		render_device->createTextures(GL_TEXTURE_2D, 4, systemTextures);

		gEngine->texture2D_black = systemTextures[0];
		gEngine->texture2D_white = systemTextures[1];
		gEngine->texture2D_grey = systemTextures[2];
		gEngine->texture2D_blue = systemTextures[3];

		cmdList.textureStorage2D(gEngine->texture2D_black, 1, GL_RGBA8, 1, 1);
		cmdList.textureStorage2D(gEngine->texture2D_white, 1, GL_RGBA8, 1, 1);
		cmdList.textureStorage2D(gEngine->texture2D_grey, 1, GL_RGBA8, 1, 1);
		cmdList.textureStorage2D(gEngine->texture2D_blue, 1, GL_RGBA8, 1, 1);

		GLubyte black[4] = { 0, 0, 0, 0 };
		GLubyte white[4] = { 0xff, 0xff, 0xff, 0xff };
		GLubyte grey[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
		GLubyte blue[4] = { 0x00, 0x00, 0xff, 0xff };

		cmdList.clearTexImage(gEngine->texture2D_black, 0, GL_RGBA, GL_UNSIGNED_BYTE, black);
		cmdList.clearTexImage(gEngine->texture2D_white, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
		cmdList.clearTexImage(gEngine->texture2D_grey, 0, GL_RGBA, GL_UNSIGNED_BYTE, grey);
		cmdList.clearTexImage(gEngine->texture2D_blue, 0, GL_RGBA, GL_UNSIGNED_BYTE, blue);

		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_black, -1, "system texture 2D (black)");
		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_white, -1, "system texture 2D (white)");
		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_grey, -1, "system texture 2D (grey)");
		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_blue, -1, "system texture 2D (blue)");

		cmdList.flushAllCommands();

		ScopedGpuCounter::initializeQueryObjectPool();

		return validDevice;
	}

	bool RenderThread::initializeOverlayRenderer() {
		renderer2D = new OverlayRenderer;

		debugOverlay = new DebugOverlay(renderer2D);
		debugOverlay->initialize();

		return true;
	}

	bool RenderThread::initializeRenderer(RenderCommandList& cmdList) {
		const auto& conf = gEngine->getConfig();
		switch (conf.rendererType) {
		case ERendererType::Forward:
			LOG(LogFatal, "Forward shading renderer is removed due to maintenance issue. Switching to deferred shading...");
			renderer = new DeferredRenderer;
			break;

		case ERendererType::Deferred:
			renderer = new DeferredRenderer;
			break;
		}

		if (renderer) {
			SceneRenderSettings settings;
			{
				settings.sceneWidth = conf.windowWidth;
				settings.sceneHeight = conf.windowHeight;
				settings.frameCounter = 0;
				settings.enablePostProcess = true;
				settings.finalRenderTarget = nullptr;
			}
			renderer->setSceneRenderSettings(settings);
			renderer->initializeResources(cmdList);
		}

		LOG(LogInfo, "Initialize scene renderer");
		return true;
	}

	bool RenderThread::destroyOpenGL() {
		ScopedGpuCounter::destroyQueryObjectPool();
		return true;
	}

	// Wait for initialization of OpenGL and rendering-related subsystems.
	void RenderThread::waitForInitialization() {
		if (!bInitialized) {
			std::unique_lock<std::mutex> cvLock(initMutex);
			initCondVar.wait(cvLock);
		}
	}

}
