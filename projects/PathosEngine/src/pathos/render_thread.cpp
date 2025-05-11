#include "render_thread.h"

#include "badger/system/cpu.h"
#include "badger/math/minmax.h"
#include "badger/types/half_float.h"
#include "badger/assertion/assertion.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/debug_overlay.h"

#include "pathos/rhi/render_device.h"
#include "pathos/rhi/gl_context_manager.h"
#include "pathos/rhi/texture.h"

#include "pathos/render/scene_proxy.h"
#include "pathos/render/renderer.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/scene_renderer.h"

#include "pathos/loader/asset_streamer.h"
#include "pathos/text/font_mgr.h"
#include "pathos/scene/scene_render_settings.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/overlay/display_object_proxy.h"

#include "pathos/util/log.h"
#include "pathos/util/cpu_profiler.h"

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
			auto& initRoutines = gEngine->internal_getGlobalRenderRoutineContainer().initRoutines;
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
			OpenGLContextManager::takeContext();
			bool bNewSceneRendered = false;

			renderThread->stopwatch.start();

			Renderer* renderer = renderThread->renderer;
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();
			RenderCommandList& earlyContext = gRenderDevice->getEarlyCommandList();
			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();

			const EngineConfig engineConfig(gEngine->getConfig());
			const int32 screenWidth = engineConfig.windowWidth;
			const int32 screenHeight = engineConfig.windowHeight;

			{
				SCOPED_CPU_COUNTER(ExecuteEarlyCommands);
				earlyContext.flushAllCommands();
			}

			GLuint64 gpu_elapsed_ns;
			immediateContext.beginQuery(GL_TIME_ELAPSED, renderThread->gpuTimerQuery);

			std::vector<SceneProxy*> sceneProxiesToDelete;

			// Render all scene proxies at once.
			while (renderThread->isSceneProxyQueueEmpty() == false) {
				SceneProxy* sceneProxy = renderThread->popSceneProxy();
				sceneProxiesToDelete.push_back(sceneProxy);

				char frameCounterMsg[128];
				sprintf_s(frameCounterMsg, "SceneProxy (source=%s frame=%u)",
					pathos::getSceneProxySourceString(sceneProxy->sceneProxySource),
					sceneProxy->frameNumber);
				SCOPED_CPU_COUNTER_STRING(frameCounterMsg);

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
						settings.frameCounter = sceneProxy->frameNumber;
						settings.enablePostProcess = true;
						renderer->setSceneRenderSettings(settings);
						renderer->setFinalRenderTargetToBackbuffer();
					}
				}

				// Renderer will add more immediate commands
				if (renderer && sceneProxy) {
					SCOPED_CPU_COUNTER(ExecuteRenderer);

					SceneRenderTargets* sceneRTs = nullptr;
					if (sceneProxy->sceneProxySource == SceneProxySource::MainScene || sceneProxy->sceneProxySource == SceneProxySource::SceneCapture) {
						sceneRTs = renderThread->sceneRenderTargets_primary;
					} else if (sceneProxy->sceneProxySource == SceneProxySource::RadianceCapture) {
						sceneRTs = renderThread->sceneRenderTargets_radianceProbe;
					} else if (sceneProxy->sceneProxySource == SceneProxySource::IrradianceCapture) {
						sceneRTs = renderThread->sceneRenderTargets_irradianceProbe;
					} else {
						CHECK_NO_ENTRY();
					}
					renderer->renderScene(immediateContext, sceneRTs, sceneProxy, &sceneProxy->camera);

					{
						char counterMsg[64];
						sprintf_s(counterMsg, "SubmitCommands (Count=%u)", immediateContext.getNumCommands());
						SCOPED_CPU_COUNTER_STRING(counterMsg);

						immediateContext.flushAllCommands();
					}

					// Update backbuffer only if main scene was actually rendered
					if (sceneProxy->sceneProxySource == SceneProxySource::MainScene) {
						bNewSceneRendered = true;
					}

					// Transfer screenshot pixels if exist.
					if (sceneProxy->bScreenshotReserved && sceneProxy->screenshotRawData.size() > 0) {
						vector2i screenshotSize = sceneProxy->screenshotSize;

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
						gEngine->internal_pushScreenshot(screenshot);
					}
				}
			} // End of scene proxies processing

			// Restore sceneRenderTargets to the one for main scene.
			immediateContext.sceneRenderTargets = renderThread->sceneRenderTargets_primary;

			// Render app UI / debug overlay / console window.
			OverlaySceneProxy* overlayProxy = renderThread->popOverlayProxy();
			if (bNewSceneRendered && overlayProxy != nullptr) {
				SCOPED_CPU_COUNTER(OverlayProxy);

				if (overlayProxy->appOverlayRootProxy != nullptr) {
					SCOPED_CPU_COUNTER(ExecuteApplicationUI);
					renderThread->getRenderer2D()->renderOverlay(
						immediateContext,
						overlayProxy->appOverlayRootProxy);
				}
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

			// Assume immediateContext is now empty here.
			{
				SCOPED_CPU_COUNTER(ExecuteDeferredCommands);
				deferredContext.flushAllCommands();
			}

			// Let subsystems handle the end of rendering.
			{
				SCOPED_CPU_COUNTER(FontManagerFrameEnd);
				FontManager::get().onFrameEnd();
			}

			// -------------------------------------------------------------------------
			// Frame rendering is done. Time to do cleanup.

			{
				// It will flush GPU so 'WaitForGPU' counter below may appear short in the profiler.
				SCOPED_CPU_COUNTER(FlushGPUAndQueryFrameTime);
				immediateContext.endQuery(GL_TIME_ELAPSED);
				immediateContext.getQueryObjectui64v(renderThread->gpuTimerQuery, GL_QUERY_RESULT, &gpu_elapsed_ns);
				immediateContext.flushAllCommands();
			}
			renderThread->elapsed_gpu = (float)gpu_elapsed_ns / 1000000.0f;

			// Get GPU profile for current frame.
			GpuCounterResult gpuCounterResult = ScopedGpuCounter::flushQueries(&immediateContext);
			renderThread->lastGpuCounterResult = std::move(gpuCounterResult);

			// Clear render resources for current frame.
			std::vector<Fence*> fencesToSignal;
			std::vector<uint64> fenceValuesToSignal;
			{
				SCOPED_CPU_COUNTER(DestroyRenderProxy);
				for (SceneProxy* sceneProxy : sceneProxiesToDelete) {
					if (sceneProxy->internal_getFence() != nullptr) {
						fencesToSignal.push_back(sceneProxy->internal_getFence());
						fenceValuesToSignal.push_back(sceneProxy->internal_getFenceValue());
					}
					delete sceneProxy;
				}
				if (overlayProxy != nullptr) {
					delete overlayProxy;
					overlayProxy = nullptr;
				}
			}

			// Pass render stats to the game thread.
			renderThread->elapsed_renderThread = renderThread->stopwatch.stop();
			if (bNewSceneRendered) {
				gEngine->internal_updateGPUQuery_renderThread(
					renderThread->elapsed_renderThread,
					renderThread->elapsed_gpu,
					renderThread->lastGpuCounterResult);
			}

			{
				SCOPED_CPU_COUNTER(WaitForGPU);
				glFlush();
			}
			OpenGLContextManager::returnContext();
			// Wait here and let GUI to take GL context and swap buffers.
			if (bNewSceneRendered) {
				SCOPED_CPU_COUNTER(Present);
				gEngine->internal_updateMainWindow_renderThread();
			}

			const size_t numFences = fencesToSignal.size();
			for (size_t i = 0; i < numFences; ++i) {
				fencesToSignal[i]->signalValue(fenceValuesToSignal[i]);
			}
		} // End of render thread loop

		// Terminate
		{
			OpenGLContextManager::takeContext();
			RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();

			// May generate render commands
			renderThread->renderer->releaseResources(cmdList);

			for (Engine::GlobalRenderRoutine routine : gEngine->internal_getGlobalRenderRoutineContainer().destroyRoutines) {
				routine(gRenderDevice, cmdList);
			}
			cmdList.flushAllCommands();

			renderThread->sceneRenderTargets_primary->freeSceneTextures(cmdList);
			renderThread->sceneRenderTargets_radianceProbe->freeSceneTextures(cmdList);
			renderThread->sceneRenderTargets_irradianceProbe->freeSceneTextures(cmdList);
			cmdList.flushAllCommands();
			SAFE_RELEASE(renderThread->sceneRenderTargets_primary);
			SAFE_RELEASE(renderThread->sceneRenderTargets_radianceProbe);
			SAFE_RELEASE(renderThread->sceneRenderTargets_irradianceProbe);

			SAFE_RELEASE(renderThread->renderer);
			SAFE_RELEASE(renderThread->renderer2D);
			SAFE_RELEASE(renderThread->debugOverlay);

			cmdList.flushAllCommands();

			bool bDestroyed = renderThread->destroyOpenGL();
			CHECKF(bDestroyed, "OpenGL not destroyed properly");

			gRenderDevice->destroyGlobalResources();

			glFlush();
			OpenGLContextManager::returnContext();
		}

		// Cleanup thread main
		{
			RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();
			RenderCommandList& earlyContext = gRenderDevice->getEarlyCommandList();
			RenderCommandList& deferredContext = gRenderDevice->getDeferredCommandList();
			CHECKF(immediateContext.isEmpty(), "Immediate command list is not empty");
			CHECKF(earlyContext.isEmpty(), "Early command list is not empty");
			CHECKF(deferredContext.isEmpty(), "Deferred command list is not empty");
		}

		renderThread->terminateCondVar.notify_all();

		gRenderDevice->reportLiveObjects();
		SAFE_RELEASE(gRenderDevice);
		renderThread->renderDevice = nullptr;

		//delete renderThread;

		LOG(LogInfo, "[%s] Render thread terminated", __FUNCTION__);
	}

	//////////////////////////////////////////////////////////

	RenderThread::RenderThread()
		: nativeThread()
		, threadID(0xffffffff)
		, threadName("Render Thread")
	{
		// Thread id and name are set in the thread entry point (renderThreadMain()).
	}

	RenderThread::~RenderThread() {
	}

	void RenderThread::run() {
		nativeThread = std::thread(renderThreadMain, this);
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
		renderDevice = new OpenGLDevice;
		bool validDevice = renderDevice->initialize();
		
		if (!validDevice) {
			return false;
		}

		RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

		renderDevice->genQueries(1, &gpuTimerQuery);
		CHECK(gpuTimerQuery != 0);

		// Create engine resources
		gEngine->geometry_unitPlane = makeAssetPtr<PlaneGeometry>(PlaneGeometry::Input{ 2.0f, 2.0f });
		gEngine->geometry_unitCube = makeAssetPtr<CubeGeometry>(vector3(1.0f));

		TextureCreateParams textureCreateParams[6] = {
			{ 1, 1, 1, 1, GL_TEXTURE_2D, GL_RGBA8, false, {}, "Texture_Black1x1" },
			{ 1, 1, 1, 1, GL_TEXTURE_2D, GL_RGBA8, false, {}, "Texture_White1x1" },
			{ 1, 1, 1, 1, GL_TEXTURE_2D, GL_RGBA8, false, {}, "Texture_Grey1x1" },
			{ 1, 1, 1, 1, GL_TEXTURE_2D, GL_RGBA8, false, {}, "Texture_Blue1x1" },
			{ 1, 1, 1, 1, GL_TEXTURE_2D, GL_RGBA8, false, {}, "Texture_Normalmap1x1" },
			{ 1, 1, 1, 1, GL_TEXTURE_CUBE_MAP, GL_RGBA8, false, {}, "Texture_BlackCube1x1" },
		};
		constexpr size_t numSystemTextures = _countof(textureCreateParams);
		Texture* systemTextures[numSystemTextures];
		for (size_t i = 0; i < numSystemTextures; ++i) {
			systemTextures[i] = new Texture(textureCreateParams[i]);
			systemTextures[i]->createGPUResource();
		}

		gEngine->texture2D_black     = systemTextures[0];
		gEngine->texture2D_white     = systemTextures[1];
		gEngine->texture2D_grey      = systemTextures[2];
		gEngine->texture2D_blue      = systemTextures[3];
		gEngine->texture2D_normalmap = systemTextures[4];
		gEngine->textureCube_black   = systemTextures[5];

		const GLubyte black[4] = { 0, 0, 0, 0 };
		const GLubyte white[4] = { 0xff, 0xff, 0xff, 0xff };
		const GLubyte grey[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
		const GLubyte blue[4] = { 0x00, 0x00, 0xff, 0xff };
		const GLubyte normalmap[4] = { 0x7f, 0x7f, 0xff, 0xff };

		cmdList.clearTexImage(gEngine->texture2D_black->internal_getGLName(), 0, GL_RGBA, GL_UNSIGNED_BYTE, black);
		cmdList.clearTexImage(gEngine->texture2D_white->internal_getGLName(), 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
		cmdList.clearTexImage(gEngine->texture2D_grey->internal_getGLName(), 0, GL_RGBA, GL_UNSIGNED_BYTE, grey);
		cmdList.clearTexImage(gEngine->texture2D_blue->internal_getGLName(), 0, GL_RGBA, GL_UNSIGNED_BYTE, blue);
		cmdList.clearTexImage(gEngine->texture2D_normalmap->internal_getGLName(), 0, GL_RGBA, GL_UNSIGNED_BYTE, normalmap);
		cmdList.clearTexImage(gEngine->textureCube_black->internal_getGLName(), 0, GL_RGBA, GL_UNSIGNED_BYTE, black);

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
		sceneRenderTargets_primary = new SceneRenderTargets;
		sceneRenderTargets_radianceProbe = new SceneRenderTargets;
		sceneRenderTargets_irradianceProbe = new SceneRenderTargets;
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
		
		gEngine->geometry_unitPlane.reset();
		gEngine->geometry_unitCube.reset();

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
