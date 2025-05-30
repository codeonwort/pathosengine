#include "engine.h"
#include "engine_version.h"
#include "console.h"
#include "render_thread.h"
#include "debug_overlay.h"

#include "pathos/rhi/gl_context_manager.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_renderer.h"
#include "pathos/render/render_overlay.h"
#include "pathos/material/material_shader_assembler.h"

#include "pathos/util/log.h"
#include "pathos/util/os_util.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/renderdoc_integration.h"

#include "pathos/scene/world.h"
#include "pathos/scene/scene.h"
#include "pathos/scene/reflection_probe_actor.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/overlay/display_object_proxy.h"
#include "pathos/overlay/display_object.h"

#include "pathos/loader/image_loader.h"   // subsystem: image loader
#include "pathos/text/font_mgr.h"         // subsystem: font manager
#include "pathos/gui/gui_window.h"        // subsystem: gui
#include "pathos/input/input_system.h"    // subsystem: input handling
#include "pathos/loader/asset_streamer.h" // subsystem: asset streamer

#include <inttypes.h>

#define CONSOLE_WINDOW_MIN_HEIGHT    400
#define ENGINE_CONFIG_FILE           "EngineConfig.ini"
#define ENGINE_CONFIG_EXTRA_FILE     "EngineConfigOverride.ini"

#define GL_DEBUG_CONTEXT             0

// Misc
namespace pathos {

	// #note: Setting this to 0 can make world delta seconds = 0.0
	//        and the world will look like frozen.
	static ConsoleVariable<int32> maxFPS("t.maxFPS", 1000, "Limit max framerate (0 = no limit)");

	Engine*        gEngine  = nullptr;
	ConsoleWindow* gConsole = nullptr;

}

// Engine (static)
namespace pathos {

	bool Engine::init(int argc, char** argv, const EngineConfig& config) {
		if (gEngine) {
			LOG(LogWarning, "The engine is already initialized");
			return false;
		}

		gEngine = new Engine;
		bool initialized = gEngine->initialize(argc, argv, config);
		CHECKF(initialized, "Failed to initialize the engine");

		return initialized;
	}

	Engine::GlobalRenderRoutineContainer& Engine::internal_getGlobalRenderRoutineContainer()
	{
		static GlobalRenderRoutineContainer instance;
		return instance;
	}

	void Engine::internal_registerGlobalRenderRoutine(GlobalRenderRoutine initRoutine, GlobalRenderRoutine destroyRoutine)
	{
		CHECK(initRoutine != nullptr);

		// Due to parallel initialization of static variables
		GlobalRenderRoutineContainer& container = internal_getGlobalRenderRoutineContainer();
		std::lock_guard<std::mutex> guard(container.vector_mutex);

		container.initRoutines.push_back(initRoutine);
		if (destroyRoutine) {
			container.destroyRoutines.push_back(destroyRoutine);
		}
	}

}

// Engine (member)
namespace pathos {

	// Initilaize the engine instance and its subsystems.
	// Init order of subsystems is carefully hard-coded. Do not reorder in random.
	bool Engine::initialize(int argc, char** argv, const EngineConfig& config) {
		conf = config;

		LOG(LogInfo, "");
		LOG(LogInfo, "===      Initialize PATHOS      ===");
		LOG(LogInfo, "Engine version: %d.%d.%d", PATHOS_MAJOR_VERSION, PATHOS_MINOR_VERSION, PATHOS_PATCH_VERSION);

		ResourceFinder::get().add("../");
		ResourceFinder::get().add("../../");
		ResourceFinder::get().add("../../shaders/");
		ResourceFinder::get().add("../../resources/");
		ResourceFinder::get().add("../../resources_external/");

		gGlobalLogFile.initialize("program_log.txt");

		std::vector<std::string> configLines;
		readConfigFile(ENGINE_CONFIG_FILE, configLines);
		readConfigFile(ENGINE_CONFIG_EXTRA_FILE, configLines);

		RenderDocIntegration::get().findInjectedDLL();

		CpuProfiler& cpuProfiler = CpuProfiler::getInstance();
		cpuProfiler.initialize();
		cpuProfiler.registerCurrentThread("main thread");

		pathos::gMainThreadId = CPU::getCurrentThreadId();

		frameFence = makeUnique<Fence>(0);
		frameNumber_mainThread = 1;

		renderThread = new RenderThread;

		// Many pesky reasons for this order :/
#define BailIfFalse(x) if(!(x)) { return false; }
		BailIfFalse( initializeMainWindow(argc, argv)          );
		// Subsystems that does not depend on the render thread.
		BailIfFalse( initializeInput()                         );
		BailIfFalse( initializeAssetStreamer()                 );
		BailIfFalse( initializeImageLibrary()                  );
		// Launch the render thread and initialize remaining subsystems that require GL context.
		renderThread->run();
		renderThread->waitForInitialization();
		FLUSH_RENDER_COMMAND();
#undef BailIfFalse

		// Register engine commands
		{
			registerConsoleCommand("profile_gpu", [](const std::string& command) {
				gEngine->dumpGPUProfile();
			});
			registerConsoleCommand("stat", [](const std::string& command) {
				gEngine->toggleFrameStat();
			});
			registerConsoleCommand("set_window_size", [](const std::string& command) {
				char unused[16];
				int w, h;
				int ret = sscanf_s(command.c_str(), "%s %d %d", unused, (unsigned)_countof(unused), &w, &h);
				if (ret == 3 && w > 0 && h > 0) {
					gEngine->getMainWindow()->setSize((uint32)w, (uint32)h);
				} else {
					gConsole->addLine(L"Usage: set_window_size <new_width> <new_height>");
				}
			});
			registerConsoleCommand("screenshot", [this](const std::string& command) {
				renderThread->takeScreenshot();
			});
			registerConsoleCommand("memreport", [](const std::string& command) {
				ENQUEUE_DEFERRED_RENDER_COMMAND([](RenderCommandList& cmdList) {
					int64 bufferMem, textureMem;
					gRenderDevice->memreport(bufferMem, textureMem);
					char msg[256];
					sprintf_s(msg, "buffer memory  : %" PRId64 " bytes (%.3lf MiB)",
						bufferMem, (double)bufferMem / (1024.0 * 1024.0));
					gConsole->addLine(msg, false, true);
					sprintf_s(msg, "texture memory : %" PRId64 " bytes (%.3lf MiB)",
						textureMem, (double)textureMem / (1024.0 * 1024.0));
					gConsole->addLine(msg, false, true);
					gConsole->addLine("Warning: For all textures, only mip0 is counted so total usage maybe bigger.", false, true);
				});
			});
		}

		for (const auto& line : configLines) {
			gConsole->addLine(line.c_str(), false);
		}

		appOverlayRoot = uniquePtr<DisplayObject2D>(DisplayObject2D::createRoot());

		MaterialShaderAssembler::get().initializeMaterialShaders();

		LOG(LogInfo, "=== PATHOS has been initialized ===");
		LOG(LogInfo, "");

		engineStatus = EngineStatus::Initialized;

		return true;
	}

	bool Engine::initializeMainWindow(int argc, char** argv) {
		GUIWindowCreateParams createParams;
		createParams.argc              = argc;
		createParams.argv              = argv;

		createParams.width             = conf.windowWidth;
		createParams.height            = conf.windowHeight;
		createParams.fullscreen        = conf.fullscreen;
		createParams.title             = conf.title;

		createParams.glMajorVersion    = REQUIRED_GL_MAJOR_VERSION;
		createParams.glMinorVersion    = REQUIRED_GL_MINOR_VERSION;
		createParams.glDebugContext    = GL_DEBUG_CONTEXT;

		createParams.onClose           = Engine::onCloseWindow;
		createParams.onIdle            = Engine::onIdle;
		createParams.onDisplay         = Engine::onMainWindowDisplay;
		createParams.onKeyDown         = Engine::onKeyDown;
		createParams.onKeyUp           = Engine::onKeyUp;
		createParams.onSpecialKeyDown  = Engine::onSpecialKeyDown;
		createParams.onSpecialKeyUp    = Engine::onSpecialKeyUp;
		createParams.onReshape         = Engine::onMainWindowReshape;
		createParams.onMouseDown       = Engine::onMouseDown;
		createParams.onMouseUp         = Engine::onMouseUp;
		createParams.onMouseDrag       = Engine::onMouseDrag;

		mainWindow = makeUnique<GUIWindow>();
		mainWindow->create(createParams);

		LOG(LogInfo, "Initialize the main window");
		return true;
	}

	bool Engine::initializeInput() {
		inputSystem = makeUnique<InputSystem>();
		return true;
	}

	bool Engine::initializeAssetStreamer() {
		assetStreamer = makeUnique<AssetStreamer>();
		return true;
	}

	bool Engine::initializeImageLibrary() {
		pathos::initializeImageLibrary();
		return true;
	}

	bool Engine::initializeFontSystem(RenderCommandList& cmdList) {
		if (FontManager::get().init() == false) {
			LOG(LogError, "[ERROR] Failed to initialize font manager");
			return false;
		}
		FontManager::get().registerFont(cmdList, "default", "resources/fonts/consola.ttf", 28);
		FontManager::get().registerFont(cmdList, "defaultLarge", "resources/fonts/consola.ttf", 84);
		FontManager::get().registerFont(cmdList, "hangul", "resources/fonts/BMJUA.ttf", 28);    // http://font.woowahan.com/jua/
		LOG(LogInfo, "Initialize font subsystem");

		return true;
	}

	bool Engine::initializeConsole() {
		gConsole = new ConsoleWindow(renderThread->getRenderer2D());
		if (gConsole->initialize(conf.windowWidth, std::min(conf.windowHeight, CONSOLE_WINDOW_MIN_HEIGHT)) == false) {
			LOG(LogError, "Failed to initialize console window");
			return false;
		}
		gConsole->addLine(L"Built-in debug console. Press ` to toggle.");

		LOG(LogInfo, "Initialize the debug console");

		return true;
	}

	void Engine::start() {
		if (engineStatus != EngineStatus::Initialized) {
			LOG(LogError, "[Engine] Engine status: %u", (uint32)engineStatus);
			return;
		}

		stopwatch_gameThread.start();
		assetStreamer->initialize(conf.numWorkersForAssetStreamer);
		renderThread->markMainLoopStarted();

		engineStatus = EngineStatus::Running;

		mainWindow->startMainLoop(); // Blocking loop
	}

	void Engine::stop(bool immediate) {
		if (engineStatus != EngineStatus::Running) {
			LOG(LogError, "[Engine] Engine status: %u", (uint32)engineStatus);
			return;
		}

		LOG(LogInfo, "");
		LOG(LogInfo, "=== Destroying PATHOS ===");

		engineStatus = EngineStatus::Destroying;

		if (immediate) {
			stopDeferred();
		}
	}

	// For scene capture
	void Engine::internal_pushSceneProxy(SceneProxy* newSceneProxy) {
		reservedSceneProxies.push_back(newSceneProxy);
	}

	void Engine::internal_pushOverlayProxy(OverlaySceneProxy* newOverlayProxy) {
		reservedOverlayProxies.push_back(newOverlayProxy);
	}

	void Engine::internal_updateMainWindow_renderThread() {
		mainWindow->updateWindow_renderThread();
	}

	void Engine::internal_updateGPUQuery_renderThread(
		float inElapsedRenderThread,
		float inElapsedGpu,
		const GpuCounterResult& inGpuCounterResult)
	{
		std::lock_guard<std::mutex> lockGuard(gpuQueryMutex);

		elapsed_renderThread = inElapsedRenderThread;
		elapsed_gpu = inElapsedGpu;
		lastGpuCounterResult = inGpuCounterResult;
	}

	void Engine::internal_updateBasePassCullStat_renderThread(uint32 totalDrawcall, uint32 culledDrawcall) {
		lastBasePassTotalDrawcall = totalDrawcall;
		lastBasePassCulledDrawcall = culledDrawcall;
	}

	// Read config line by line and add to the console window.
	void Engine::readConfigFile(const char* configFilename, std::vector<std::string>& outEffectiveLines)
	{
		std::string configPath = ResourceFinder::get().find(configFilename);
		if (configPath.size() > 0) {
			LOG(LogInfo, "Read config file: %s", configPath.c_str());

			std::fstream file(configPath);
			std::string line;
			while (std::getline(file, line)) {
				// Lines that are empty or start with # are comments
				if (line.size() == 0 || line[0] == '#') {
					continue;
				}

				auto ix = line.find(' ');
				if (ix != std::string::npos) {
					std::string cvarName = line.substr(0, ix);
					std::string cvarValue = line.substr(ix + 1);
					auto cvar = ConsoleVariableManager::get().find(cvarName.data());
					if (cvar != nullptr) {
						cvar->parse(cvarValue.data(), nullptr);
					}
				}
				
				outEffectiveLines.emplace_back(line);
			}
		}
	}

	void Engine::registerConsoleCommand(const char* command, ExecProc proc)
	{
		if (execMap.find(command) != execMap.end()) {
			LOG(LogError, "Exec already registered: %s", command);
			return;
		}
		execMap.insert(std::make_pair(command, proc));
	}

	bool Engine::executeConsoleCommand(const std::string& command)
	{
		auto ix = command.find(' ');
		std::string header = ix == std::string::npos ? command : command.substr(0, ix);

		auto it = execMap.find(header);
		if (it != execMap.end()) {
			it->second(command);
			return true;
		}

		return false;
	}

	void Engine::toggleFrameStat() {
		renderThread->getDebugOverlay()->toggleFrameStat();
	}

	// #todo-stat: Show this in debug GUI
	void Engine::dumpGPUProfile()
	{
		std::lock_guard<std::mutex> lockGuard(gpuQueryMutex);

		std::string filepath = pathos::getSolutionDir();
		filepath += "log/";
		pathos::createDirectory(filepath.c_str());

		time_t now = ::time(0);
		tm localTm;
		errno_t timeErr = ::localtime_s(&localTm, &now);
		CHECKF(timeErr == 0, "Failed to get current time");
		char timeBuffer[128];
		::strftime(timeBuffer, sizeof(timeBuffer), "GPUProfile-%Y-%m-%d-%H-%M-%S.txt", &localTm);
		filepath += std::string(timeBuffer);

		const auto& counterNames = lastGpuCounterResult.counterNames;
		const auto& counterTimes = lastGpuCounterResult.elapsedMilliseconds;
		const auto& indentLevels = lastGpuCounterResult.indentLevels;

		int32 maxNameLen = 0;
		for (size_t i = 0; i <lastGpuCounterResult.numCounters; ++i)
		{
			int32 paddedLen = (int32)counterNames[i].size() + indentLevels[i];
			if (paddedLen > maxNameLen) maxNameLen = paddedLen;
		}

		std::fstream fs(filepath, std::fstream::out);
		if (fs.is_open()) {
			uint32 n = (uint32)lastGpuCounterResult.numCounters;
			for (uint32 i = 0; i < n; ++i) {
				for (int32 space = 0; space < indentLevels[i]; ++space) fs << ' ';
				fs << counterNames[i];
				int32 postSpace = maxNameLen - (int32)counterNames[i].size() - (int32)indentLevels[i];
				for (size_t space = 0; space <= postSpace; ++space) fs << ' ';
				fs << ": " << counterTimes[i] << " ms" << std::endl;
			}
			fs.close();
		}

		LOG(LogDebug, "Dump GPU profile to: %s", filepath.c_str());
		gConsole->addLine("Dump GPU profile to:", false);
		gConsole->addLine(filepath.c_str(), false);
	}

	void Engine::internal_getLastGPUCounters(
		std::vector<std::string>& outGpuCounterNames,
		std::vector<float>& outGpuCounterTimes)
	{
		std::lock_guard<std::mutex> lockGuard(gpuQueryMutex);
		outGpuCounterNames = lastGpuCounterResult.counterNames;
		outGpuCounterTimes = lastGpuCounterResult.elapsedMilliseconds;
	}

	void Engine::setWorld(World* inWorld) {
		pendingNewWorld = inWorld;
	}

	assetPtr<MeshGeometry> Engine::getSystemGeometryUnitPlane() const {
		return geometry_unitPlane;
	}

	assetPtr<MeshGeometry> Engine::getSystemGeometryUnitCube() const {
		return geometry_unitCube;
	}

	void Engine::internal_updateScreenSize(int32 inScreenWidth, int32 inScreenHeight) {
		conf.windowWidth = inScreenWidth;
		conf.windowHeight = inScreenHeight;
	}

	void Engine::internal_pushScreenshot(Screenshot screenshot) {
		screenshotQueue.emplace_back(screenshot);
	}

	void Engine::tickMainThread() {
		if (frameFence->getValue() + 2 < frameNumber_mainThread) {
			return;
		}

		char frameCounterMsg[256];
		sprintf_s(frameCounterMsg, "Frame %u", frameNumber_mainThread);
		SCOPED_CPU_COUNTER_STRING(frameCounterMsg);

		float deltaSeconds = 0.001f * stopwatch_gameThread.stop();
		if (maxFPS.getValue() > 0 && deltaSeconds < 1.0f / maxFPS.getValue()) {
			return;
		} else {
			stopwatch_gameThread.start();
		}

		CpuProfiler::getInstance().beginCheckpoint(frameNumber_mainThread);

		// Change world if necessary.
		if (pendingNewWorld != nullptr) {
			// As current world might destroy GL resources on its destruction,
			// make sure no GPU work is in flight.
			{
				LOG(LogDebug, "Flush render thread before world transition");

				uint64 renderFrameNumber = frameNumber_mainThread - 1;
				if (frameFence->getValue() < renderFrameNumber) {
					SCOPED_CPU_COUNTER(WaitForRenderThread);

					SyncEvent syncEvent;
					frameFence->setEventOnCompletion(renderFrameNumber, &syncEvent);
					syncEvent.waitInfinite();
					syncEvent.close();
				}
			}

			if (currentWorld != nullptr) {
				currentWorld->destroy();
				delete currentWorld;
			}
			currentWorld = pendingNewWorld;
			currentWorld->initialize();

			pendingNewWorld = nullptr;

			// #todo-asset-streamer: Pending works for old world should be cancelled.
			assetStreamer->wakeThreadPool();
		}

		// Start world tick.
		if (engineStatus == EngineStatus::Running)
		{
			SCOPED_CPU_COUNTER(WorldTick);

			{
				SCOPED_CPU_COUNTER(InputTick);
				inputSystem->tick();
			}

			{
				SCOPED_CPU_COUNTER(FlushLoadedAssets);
				getAssetStreamer()->internal_flushLoadedAssets();
			}

			if (currentWorld != nullptr) {
				const float ar = (float)conf.windowWidth / conf.windowHeight;
				currentWorld->getCamera().getLens().setAspectRatio(ar);

				{
					SCOPED_CPU_COUNTER(UpdateCurrentWorld);
					currentWorld->tick(deltaSeconds);
				}

				{
					SCOPED_CPU_COUNTER(CreateSceneProxies);

					// Process probe lighting.
					{
						currentWorld->getScene().updateLightProbes();
					}

					// Render the main view.
					{
						SCOPED_CPU_COUNTER(MainSceneProxy);

						SceneProxyCreateParams sceneProxyParams{
							SceneProxySource::MainScene,
							frameNumber_mainThread,
							currentWorld->getCamera(),
							frameFence.get(),
							frameNumber_mainThread,
						};
						SceneProxy* mainSceneProxy = currentWorld->getScene().createRenderProxy(sceneProxyParams);
						CHECK(mainSceneProxy != nullptr);

						internal_pushSceneProxy(mainSceneProxy);
						//renderThread->pushSceneProxy(mainSceneProxy);
					}
				}
			}

			//
			// UI tick
			//
			{
				SCOPED_CPU_COUNTER(CreateOverlayProxy);

				DisplayObject2D* debugOverlayRoot = renderThread->getDebugOverlay()->internal_getRoot();
				DisplayObject2D* consoleWindowRoot = gConsole->internal_getRoot();

				OverlaySceneProxy* overlayProxy = new OverlaySceneProxy(conf.windowWidth, conf.windowHeight);
				overlayProxy->appOverlayRootProxy = DisplayObject2D::createRenderProxyHierarchy(appOverlayRoot.get(), overlayProxy);
				overlayProxy->debugOverlayRootProxy = DisplayObject2D::createRenderProxyHierarchy(debugOverlayRoot, overlayProxy);
				overlayProxy->consoleWindowRootProxy = DisplayObject2D::createRenderProxyHierarchy(consoleWindowRoot, overlayProxy);
				internal_pushOverlayProxy(overlayProxy);
			}

			//
			// Output screenshots
			//
			if (screenshotQueue.size() > 0) {
				std::string screenshotDir = pathos::getSolutionDir() + "/log/screenshot/";
				pathos::createDirectory(screenshotDir.c_str());
				for (size_t i = 0; i < screenshotQueue.size(); ++i) {
					std::string screenshotPath = screenshotDir;

					time_t now = ::time(0);
					tm localTm;
					errno_t timeErr = ::localtime_s(&localTm, &now);
					CHECKF(timeErr == 0, "Failed to get current time");
					char timeBuffer[128];
					::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d-%H-%M-%S", &localTm);

					screenshotPath += std::string(timeBuffer);
					screenshotPath += "_shot" + std::to_string(i) + ".png";
					const vector2i& size = screenshotQueue[i].first;
					uint8* pixels = screenshotQueue[i].second;
					ImageUtils::saveRGB8ImageAsPNG(size.x, size.y, pixels, screenshotPath.c_str());
					delete[] pixels;
				}
				gConsole->addLine(L"Screenshot saved to log/screenshot/", false, true);
				screenshotQueue.clear();
			}
		} // End of world tick

		CpuProfiler::getInstance().finishCheckpoint();

		elapsed_gameThread = stopwatch_gameThread.stop();
		//stopwatch_gameThread.start(); // Should not reset timer here?

		// Wait for previous frame's rendering to finish.
		uint64 renderFrameNumber = frameNumber_mainThread - 1;
		if (frameFence->getValue() < renderFrameNumber) {
			SCOPED_CPU_COUNTER(WaitForRenderThread);

			SyncEvent syncEvent;
			frameFence->setEventOnCompletion(renderFrameNumber, &syncEvent);
			syncEvent.waitInfinite();
			syncEvent.close();
		}

		if (reservedSceneProxies.size() > 0) {
			for (SceneProxy* proxy : reservedSceneProxies) {
				renderThread->pushSceneProxy(proxy);
			}
			reservedSceneProxies.clear();
		}
		if (reservedOverlayProxies.size() > 0) {
			for (OverlaySceneProxy* proxy : reservedOverlayProxies) {
				renderThread->pushOverlayProxy(proxy);
			}
			reservedOverlayProxies.clear();
		}

		// Increase frame number.
		if (frameNumber_mainThread == (uint32)(-1)) {
			frameNumber_mainThread = 1;
		} else {
			frameNumber_mainThread += 1;
		}
	}

	void Engine::stopDeferred() {
		if (engineStatus != EngineStatus::Destroying) {
			return;
		}

		if (currentWorld != nullptr) {
			currentWorld->destroy();
			delete currentWorld;
		}

		mainWindow->stopMainLoop();

		assetStreamer->destroy();
		pathos::destroyImageLibrary();

		renderThread->terminate();

		appOverlayRoot.reset();

		LOG(LogInfo, "=== PATHOS has been destroyed ===");
		LOG(LogInfo, "");

		engineStatus = EngineStatus::Destroyed;
	}

	/////////////////////////////////////////////////////////////////////
	// GUI callback functions
	/////////////////////////////////////////////////////////////////////

	void Engine::onCloseWindow() {
		gEngine->stop(true);
	}

	void Engine::onIdle() {
		if (gEngine->engineStatus == EngineStatus::Destroying) {
			gEngine->stopDeferred();
		} else {
			gEngine->tickMainThread();
		}
	}

	void Engine::onMainWindowDisplay() {
		//gEngine->tickMainThread();
	}

	void Engine::onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY) {
		constexpr uint8 BACKTICK = 0x60;
		if (ascii == BACKTICK) {
			// backtick
			gConsole->toggle();
		} else if (gConsole->isVisible()) {
			gConsole->onKeyPress(ascii);
		} else {
			gEngine->inputSystem->processRawKeyDown(ascii);
		}
	}

	void Engine::onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY) {
		gEngine->inputSystem->processRawKeyUp(ascii);
	}

	void Engine::onMainWindowReshape(int32 newWidth, int32 newHeight) {
		gEngine->internal_updateScreenSize(newWidth, newHeight);
	}

	void Engine::onSpecialKeyDown(InputConstants specialKey) {
		if (gConsole->isVisible()) {
			if (specialKey == InputConstants::KEYBOARD_ARROW_UP) {
				gConsole->showPreviousHistory();
			} else if (specialKey == InputConstants::KEYBOARD_ARROW_DOWN) {
				gConsole->showNextHistory();
			}
		} else {
			gEngine->inputSystem->processSpecialKeyDown(specialKey);
		}
	}

	void Engine::onSpecialKeyUp(InputConstants specialKey) {
		gEngine->inputSystem->processSpecialKeyUp(specialKey);
	}

	void Engine::onMouseDown(InputConstants mouseInput, int32 mouseX, int32 mouseY) {
		if (gConsole->isVisible() && mouseInput == InputConstants::MOUSE_MIDDLE_BUTTON) {
			std::wstring pasteMsg;
			if (pathos::getOSClipboardUnicodeText(pasteMsg)) {
				gConsole->appendInput(pasteMsg);
			}
			return;
		}

		gEngine->inputSystem->processButtonDown(mouseInput);

		if (mouseInput == InputConstants::MOUSE_LEFT_BUTTON) {
			gEngine->getOverlayRoot()->handleMouseLeftClick(mouseX, mouseY);
		}
	}

	void Engine::onMouseUp(InputConstants mouseInput, int32 mouseX, int32 mouseY) {
		gEngine->inputSystem->processButtonUp(mouseInput);
	}

	void Engine::onMouseDrag(int32 mouseX, int32 mouseY) {
		gEngine->getOverlayRoot()->handleMouseDrag(mouseX, mouseY);
	}

}
