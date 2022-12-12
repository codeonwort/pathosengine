#include "render_thread.h"

#include "badger/thread/cpu.h"
#include "badger/math/minmax.h"
#include "badger/assertion/assertion.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/debug_overlay.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/text/font_mgr.h"
#include "pathos/scene/scene_render_settings.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/renderer.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/scene_renderer.h"
#include "pathos/util/log.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/rhi/gl_context_manager.h"
#include "pathos/overlay/display_object_proxy.h"

#define SAFE_RELEASE(x) { if (x) delete x; x = nullptr; }

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

			renderThread->stopwatch.start();

			Renderer* renderer = renderThread->renderer;
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();
			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();

			const EngineConfig engineConfig(gEngine->getConfig());
			const int32 screenWidth = engineConfig.windowWidth;
			const int32 screenHeight = engineConfig.windowHeight;

			GLuint64 gpu_elapsed_ns;
			immediateContext.beginQuery(GL_TIME_ELAPSED, renderThread->gpuTimerQuery);

			std::vector<SceneProxy*> sceneProxiesToDelete;

			while (renderThread->isSceneProxyQueueEmpty() == false) {
				SceneProxy* sceneProxy = renderThread->popSceneProxy();
				sceneProxiesToDelete.push_back(sceneProxy);

				RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
				char drawEventMsg[64];
				sprintf_s(drawEventMsg, "RenderSceneProxy (source=%s)", pathos::getSceneProxySourceString(sceneProxy->sceneProxySource));
				SCOPED_DRAW_EVENT_STRING(drawEventMsg);

				bool bMainScene = (sceneProxy == nullptr) || (sceneProxy->sceneProxySource == SceneProxySource::MainScene);

				if (bMainScene && sceneProxy != nullptr && renderThread->bScreenshotReserved) {
					renderThread->bScreenshotReserved = false;
					sceneProxy->bScreenshotReserved = true;
				}

				if (renderer) {
					if (sceneProxy && sceneProxy->bSceneRenderSettingsOverriden) {
						// Probably scene capture
						renderer->setSceneRenderSettings(sceneProxy->sceneRenderSettingsOverride);
					} else {
						// Main scene
						SceneRenderSettings settings;
						settings.sceneWidth = screenWidth;
						settings.sceneHeight = screenHeight;
						settings.frameCounter = renderThread->currentFrame;
						settings.enablePostProcess = true;
						renderer->setSceneRenderSettings(settings);
						renderer->setFinalRenderTargetToBackbuffer();
					}
				}

				deferredContext.flushAllCommands();

				// Renderer will add more immediate commands
				if (renderer && sceneProxy) {
					SCOPED_CPU_COUNTER(ExecuteRenderer);
					renderer->render(immediateContext, sceneProxy, &sceneProxy->camera);
					immediateContext.flushAllCommands();
					//deferredContext.flushAllCommands();

					// Update backbuffer only if main scene was actually rendered
					if (sceneProxy->sceneProxySource == SceneProxySource::MainScene) {
						bNewSceneRendered = true;
					}

					// Transfer screenshot pixels if exist.
					if (sceneProxy->bScreenshotReserved && sceneProxy->screenshotRawData.size() > 0) {
						vector2i screenshotSize = sceneProxy->screenshotSize;

						// https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
						auto as_uint = [](const float x) -> uint32 { return *(uint32*)&x; };
						auto as_float = [](const uint32 x) -> float { return *(float*)&x; };
						auto half_to_float = [as_uint, as_float](const uint16 x) -> float { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
							const uint32 e = (x & 0x7C00) >> 10; // exponent
							const uint32 m = (x & 0x03FF) << 13; // mantissa
							const uint32 v = as_uint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
							return as_float((x & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
						};
						auto float_to_half = [as_uint](const float x) -> uint16 { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
							const uint32 b = as_uint(x) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
							const uint32 e = (b & 0x7F800000) >> 23; // exponent
							const uint32 m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
							return (b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
						};
						// Convert float16 to float32.
						const int32 totalPixels = screenshotSize.x * screenshotSize.y;
						const std::vector<uint16>& rawPixels = sceneProxy->screenshotRawData;
						uint8* pixels = new uint8[totalPixels * 3];
						for (int32 i = 0; i < totalPixels; ++i) {
							float R = half_to_float(rawPixels[i * 4 + 0]);
							float G = half_to_float(rawPixels[i * 4 + 1]);
							float B = half_to_float(rawPixels[i * 4 + 2]);
							pixels[i * 3 + 0] = (uint8)badger::clamp(0u, (uint32)(B * 255.0f), 255u);
							pixels[i * 3 + 1] = (uint8)badger::clamp(0u, (uint32)(G * 255.0f), 255u);
							pixels[i * 3 + 2] = (uint8)badger::clamp(0u, (uint32)(R * 255.0f), 255u);
						}
						auto screenshot = std::make_pair(sceneProxy->screenshotSize, pixels);
						gEngine->pushScreenshot(screenshot);
					}
				}
			}

			// Render debug overlay and command console
			OverlaySceneProxy* overlayProxy = renderThread->popOverlayProxy();
			if (bNewSceneRendered && overlayProxy != nullptr) {
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

			{
				SCOPED_CPU_COUNTER(ExecuteCommands);
				immediateContext.flushAllCommands();
				deferredContext.flushAllCommands();
			}
			renderThread->elapsed_gpu = (float)gpu_elapsed_ns / 1000000.0f;

			// Get GPU profile
			const uint32 numGpuCounters = ScopedGpuCounter::flushQueries(
				&immediateContext,
				renderThread->lastGpuCounterNames,
				renderThread->lastGpuCounterTimes);

			// Clear render resources for current frame.
			{
				SCOPED_CPU_COUNTER(DestroyRenderProxy);
				for (SceneProxy* sceneProxy : sceneProxiesToDelete) {
					delete sceneProxy;
				}
				if (overlayProxy != nullptr) {
					delete overlayProxy;
					overlayProxy = nullptr;
				}
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
			{
				SCOPED_CPU_COUNTER(WaitForGPU);
				glFinish();
			}
			OpenGLContextManager::returnContext();
			// Wait here and let GUI to take GL context and swap buffers.
			if (bNewSceneRendered) {
				SCOPED_CPU_COUNTER(Present);
				gEngine->updateMainWindow_renderThread();
			}
		} // End of render thread loop

		// Terminate
		{
			OpenGLContextManager::takeContext();
			RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();

			// May generate render commands
			renderThread->renderer->releaseResources(cmdList);

			for (Engine::GlobalRenderRoutine routine : gEngine->getGlobalRenderRoutineContainer().destroyRoutines) {
				routine(gRenderDevice, cmdList);
			}
			cmdList.flushAllCommands();

			SAFE_RELEASE(renderThread->renderer);
			SAFE_RELEASE(renderThread->renderer2D);
			SAFE_RELEASE(renderThread->debugOverlay);

			cmdList.flushAllCommands();

			bool bDestroyed = renderThread->destroyOpenGL();
			CHECKF(bDestroyed, "OpenGL not destroyed properly");

			glFlush();
			OpenGLContextManager::returnContext();
		}

		// Cleanup thread main
		{
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();
			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();
			CHECKF(immediateContext.isEmpty(), "Immediate command list is not empty");
			CHECKF(deferredContext.isEmpty(), "Deferred command list is not empty");
		}

		renderThread->terminateCondVar.notify_all();

		SAFE_RELEASE(renderThread->render_device);

		//delete renderThread;

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
		bPendingKill = true;
		loopCondVar.notify_all();

		std::unique_lock<std::mutex> cvLock(terminateMutex);
		terminateCondVar.wait(cvLock);

		LOG(LogInfo, "Destroy render thread");
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
		GLuint systemTextures[5];
		render_device->createTextures(GL_TEXTURE_2D, 5, systemTextures);

		gEngine->texture2D_black = systemTextures[0];
		gEngine->texture2D_white = systemTextures[1];
		gEngine->texture2D_grey = systemTextures[2];
		gEngine->texture2D_blue = systemTextures[3];
		gEngine->texture2D_normalmap = systemTextures[4];

		cmdList.textureStorage2D(gEngine->texture2D_black, 1, GL_RGBA8, 1, 1);
		cmdList.textureStorage2D(gEngine->texture2D_white, 1, GL_RGBA8, 1, 1);
		cmdList.textureStorage2D(gEngine->texture2D_grey, 1, GL_RGBA8, 1, 1);
		cmdList.textureStorage2D(gEngine->texture2D_blue, 1, GL_RGBA8, 1, 1);
		cmdList.textureStorage2D(gEngine->texture2D_normalmap, 1, GL_RGBA8, 1, 1);

		GLubyte black[4] = { 0, 0, 0, 0 };
		GLubyte white[4] = { 0xff, 0xff, 0xff, 0xff };
		GLubyte grey[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
		GLubyte blue[4] = { 0x00, 0x00, 0xff, 0xff };
		GLubyte normalmap[4] = { 0x7f, 0x7f, 0xff, 0xff };

		cmdList.clearTexImage(gEngine->texture2D_black, 0, GL_RGBA, GL_UNSIGNED_BYTE, black);
		cmdList.clearTexImage(gEngine->texture2D_white, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
		cmdList.clearTexImage(gEngine->texture2D_grey, 0, GL_RGBA, GL_UNSIGNED_BYTE, grey);
		cmdList.clearTexImage(gEngine->texture2D_blue, 0, GL_RGBA, GL_UNSIGNED_BYTE, blue);
		cmdList.clearTexImage(gEngine->texture2D_normalmap, 0, GL_RGBA, GL_UNSIGNED_BYTE, normalmap);

		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_black, -1, "system texture 2D (black)");
		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_white, -1, "system texture 2D (white)");
		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_grey, -1, "system texture 2D (grey)");
		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_blue, -1, "system texture 2D (blue)");
		cmdList.objectLabel(GL_TEXTURE, gEngine->texture2D_normalmap, -1, "system texture 2D (normalmap)");

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
		renderer = new SceneRenderer;

		const EngineConfig& conf = gEngine->getConfig();
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
