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

namespace pathos {

	void RenderThread::renderThreadMain(RenderThread* renderThread) {
		renderThread->threadID = CPU::getCurrentThreadId();
		pathos::gRenderThreadId = renderThread->threadID;
		CPU::setCurrentThreadName(L"Render Thread");

		CpuProfiler& cpuProfiler = CpuProfiler::getInstance();
		cpuProfiler.registerCurrentThread("render thread");

		// Without this, calling FLUSH_RENDER_COMMAND() before the main loop causes deadlock.
		while (!renderThread->mainLoopStarted) {
			SCOPED_TAKE_GL_CONTEXT();
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();
			immediateContext.flushAllCommands();

			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();
			deferredContext.flushAllCommands();
		}

		while (renderThread->pendingKill == false) {
			// 1. Main thread inform the render thread to render frame N.
			// 2. Main thread do game tick for frame (N+1).
			// 3. Main thread create render proxies for frame (N+1).
			// 4. Main thread waits for render thread to finish rendering frame N.
			// 5. Flip the backbuffer and go to the next frame.

			//
			// Wait until the game thread launches the render thread.
			//
			// #todo-renderthread-fatal: deadlock if main thread waits for render thread!!!
			std::unique_lock<std::mutex> cvLock(renderThread->loopMutex);
			renderThread->loopCondVar.wait(cvLock);

			//
			// Start a frame!
			//
			OpenGLContextManager::takeContext();
			renderThread->endFrameMarker = false;

			SceneProxy* sceneProxy = renderThread->popSceneProxy();
			Renderer* renderer = renderThread->renderer;;
			OverlayRenderer* renderer2D = renderThread->renderer2D;
			DebugOverlay* debugOverlay = renderThread->debugOverlay;
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();

			GLuint64 gpu_elapsed_ns;
			immediateContext.beginQuery(GL_TIME_ELAPSED, renderThread->gpuTimerQuery);

			renderThread->stopwatch.start();

			SCOPED_CPU_COUNTER(EngineRender);

			{
				SCOPED_CPU_COUNTER(FlushLoadedAssets);
				gEngine->getAssetStreamer()->renderThread_flushLoadedAssets();
			}

			// #todo-renderthread-fatal: How to do updateDynamicData_renderThread()
			{
				//currentWorld->getScene().updateDynamicData_renderThread(immediateContext);
			}

			const EngineConfig engineConfig(gEngine->getConfig());
			if (renderer) {
				SceneRenderSettings settings;
				settings.sceneWidth = engineConfig.windowWidth; // #todo: Current window size
				settings.sceneHeight = engineConfig.windowHeight;
				settings.frameCounter = renderThread->currentFrame;
				settings.enablePostProcess = true;
				renderer->setSceneRenderSettings(settings);
			}

			// #todo-cmd-list: deferred command lists here
			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();
			deferredContext.flushAllCommands();

			// Renderer will add more immediate commands
			// #todo-renderthread-fatal: Crashes at checkFramebufferStatus()
			if (false && renderer && sceneProxy)
			{
				SCOPED_CPU_COUNTER(ExecuteRenderer);
				renderer->render(immediateContext, sceneProxy, &sceneProxy->camera);
				immediateContext.flushAllCommands();

#define FIXME_OVERLAY_RENDERING 1
#if FIXME_OVERLAY_RENDERING
				// #todo-renderthread-fatal: sceneRenderTargets invalid until DeferredRenderer::render() is not executed.
				debugOverlay->renderDebugOverlay(immediateContext, engineConfig.windowWidth, engineConfig.windowHeight);
				immediateContext.flushAllCommands();

				if (gConsole) {
					SCOPED_CPU_COUNTER(ExecuteDebugConsole);
					gConsole->renderConsoleWindow(immediateContext);
					immediateContext.flushAllCommands();
				}
#endif
			}

#if !FIXME_OVERLAY_RENDERING
			debugOverlay->renderDebugOverlay(immediateContext, engineConfig.windowWidth, engineConfig.windowHeight);
			immediateContext.flushAllCommands();
			
			if (gConsole)
			{
				SCOPED_CPU_COUNTER(ExecuteDebugConsole);
				gConsole->renderConsoleWindow(immediateContext);
				immediateContext.flushAllCommands();
			}
#endif

			immediateContext.endQuery(GL_TIME_ELAPSED);
			immediateContext.getQueryObjectui64v(renderThread->gpuTimerQuery, GL_QUERY_RESULT, &gpu_elapsed_ns);
			immediateContext.flushAllCommands();
			renderThread->elapsed_gpu = (float)gpu_elapsed_ns / 1000000.0f;

			// Get GPU profile
			const uint32 numGpuCounters = ScopedGpuCounter::flushQueries(immediateContext, renderThread->lastGpuCounterNames, renderThread->lastGpuCounterTimes);

			// Clear various render resources after all rendering is done
			if (sceneProxy != nullptr) {
				delete sceneProxy;
				sceneProxy = nullptr;
			}

			FontManager::get().onFrameEnd();

			renderThread->elapsed_renderThread = renderThread->stopwatch.stop() * 1000.0f;

			//
			// End a frame
			//
			OpenGLContextManager::returnContext();
			renderThread->endFrameCondVar.notify_all();
			renderThread->endFrameMarker = true;
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
		, threadName("render_thread")
		, render_device(nullptr)
		, renderer(nullptr)
		, renderer2D(nullptr)
		, debugOverlay(nullptr)
		, currentFrame(0)
		, elapsed_renderThread(0.0f)
		, gpuTimerQuery(0)
		, elapsed_gpu(0.0f)
	{
		// #todo-renderthread: Set thread id and name (STL? Win32?)
	}

	RenderThread::~RenderThread() {
	}

	void RenderThread::run() {
		nativeThread = std::thread(renderThreadMain, this);
	}

	//void RenderThread::waitForInitialization() {
	//	if (!initialized) {
	//		std::unique_lock<std::mutex> cvLock(initMutex);
	//		initCondVar.wait(cvLock);
	//	}
	//}

	void RenderThread::beginFrame(uint32 frameNumber) {
		currentFrame = frameNumber;
		loopCondVar.notify_all();
	}

	void RenderThread::endFrame(uint32 frameNumber) {
		CHECKF(currentFrame == frameNumber, "Frame number does not match!!!");
		if (!pendingKill && !endFrameMarker) {
			std::unique_lock<std::mutex> cvLock(endFrameMutex);
			endFrameCondVar.wait(cvLock);
		}
	}

	void RenderThread::terminate() {
		for (Engine::GlobalRenderRoutine routine : gEngine->getGlobalRenderRoutineContainer().destroyRoutines) {
			routine(gRenderDevice, gRenderDevice->getDeferredCommandList());
		}

		// #todo-renderthread: Does this ensure the lock will break?
		pendingKill = true;
		loopCondVar.notify_all();
	}
	
	bool RenderThread::isSceneProxyQueueEmpty() {
		std::lock_guard<std::mutex> guard(sceneProxyQueueMutex);
		return sceneProxyQueue.empty();
	}

	SceneProxy* RenderThread::popSceneProxy() {
		std::lock_guard<std::mutex> guard(sceneProxyQueueMutex);
		if (sceneProxyQueue.empty()) {
			return nullptr;
		}
		SceneProxy* sceneProxy = sceneProxyQueue.front();
		sceneProxyQueue.pop();
		return sceneProxy;
	}

	void RenderThread::pushSceneProxy(SceneProxy* inSceneProxy) {
		std::lock_guard<std::mutex> guard(sceneProxyQueueMutex);
		sceneProxyQueue.push(inSceneProxy);
	}

	bool RenderThread::initializeOpenGL() {
		SCOPED_TAKE_GL_CONTEXT();

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

		// #todo-renderthread-fatal: glFinish to empty GPU works and safely detach the GL context from Main Thread.
		glFinish();

		return validDevice;
	}

	bool RenderThread::initializeOverlayRenderer() {
		auto This = this;
		ENQUEUE_RENDER_COMMAND([This](RenderCommandList& cmdList) {
			This->renderer2D = new OverlayRenderer;

			This->debugOverlay = new DebugOverlay(This->renderer2D);
			This->debugOverlay->initialize();
		});
		FLUSH_RENDER_COMMAND();
		return true;
	}

	bool RenderThread::initializeRenderer() {
		auto This = this;
		ENQUEUE_RENDER_COMMAND([This](RenderCommandList& cmdList) {
			const auto& conf = gEngine->getConfig();
			switch (conf.rendererType) {
			case ERendererType::Forward:
				LOG(LogFatal, "Forward shading renderer is removed due to maintenance issue. Switching to deferred shading...");
				This->renderer = new DeferredRenderer;
				break;

			case ERendererType::Deferred:
				This->renderer = new DeferredRenderer;
				break;
			}

			if (This->renderer) {
				SceneRenderSettings settings;
				{
					settings.sceneWidth = conf.windowWidth;
					settings.sceneHeight = conf.windowHeight;
					settings.enablePostProcess = true;
				}
				This->renderer->setSceneRenderSettings(settings);
				This->renderer->initializeResources(cmdList);
			}
		});
		FLUSH_RENDER_COMMAND();
		return true;
	}

}
