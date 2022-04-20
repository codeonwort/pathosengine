#include "engine.h"
#include "engine_version.h"
#include "console.h"
#include "pathos/actor/world.h"
#include "pathos/scene/scene.h"
#include "pathos/render/render_device.h"
#include "pathos/render/render_deferred.h"
#include "pathos/render/render_overlay.h"
#include "pathos/thread/render_thread.h"
#include "pathos/util/log.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/gl_context_manager.h"
#include "pathos/util/renderdoc_integration.h"
#include "pathos/debug_overlay.h"

#include "pathos/loader/imageloader.h"    // subsystem: image loader
#include "pathos/text/font_mgr.h"         // subsystem: font manager
#include "pathos/gui/gui_window.h"        // subsystem: gui
#include "pathos/input/input_system.h"    // subsystem: input
#include "pathos/loader/asset_streamer.h" // subsystem: asset streamer

#define CONSOLE_WINDOW_MIN_HEIGHT 400

namespace pathos {

	static ConsoleVariable<int32> maxFPS("t.maxFPS", 0, "Limit max framerate (0 = no limit)");

	Engine*        gEngine  = nullptr;
	ConsoleWindow* gConsole = nullptr;

	//////////////////////////////////////////////////////////////////////////

	// static
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

	Engine::GlobalRenderRoutineContainer& Engine::getGlobalRenderRoutineContainer()
	{
		static GlobalRenderRoutineContainer instance;
		return instance;
	}


	void Engine::internal_registerGlobalRenderRoutine(GlobalRenderRoutine initRoutine, GlobalRenderRoutine destroyRoutine)
	{
		CHECK(initRoutine != nullptr);

		// Due to parallel initialization of static variables
		GlobalRenderRoutineContainer& container = getGlobalRenderRoutineContainer();
		std::lock_guard<std::mutex> guard(container.vector_mutex);

		container.initRoutines.push_back(initRoutine);
		if (destroyRoutine) {
			container.destroyRoutines.push_back(destroyRoutine);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Engine::Engine()
		: frameCounter_gameThread(0)
		, elapsed_gameThread(0.0f)
		, elapsed_renderThread(0.0f)
		, currentWorld(nullptr)
		, renderThread(nullptr)
		, elapsed_gpu(0)
	{
	}

	Engine::~Engine()
	{
	}

	bool Engine::initialize(int argc, char** argv, const EngineConfig& config) {
		conf = config;

		LOG(LogInfo, "");
		LOG(LogInfo, "===      Initialize PATHOS      ===");
		LOG(LogInfo, "Engine version: %d.%d.%d", PATHOS_MAJOR_VERSION, PATHOS_MINOR_VERSION, PATHOS_PATCH_VERSION);

		ResourceFinder::get().add("../");
		ResourceFinder::get().add("../../");
		ResourceFinder::get().add("../../shaders/");
		ResourceFinder::get().add("../../resources/");

		RenderDocIntegration::get().findInjectedDLL();

		CpuProfiler& cpuProfiler = CpuProfiler::getInstance();
		cpuProfiler.initialize();
		cpuProfiler.registerCurrentThread("main thread");

		pathos::gMainThreadId = CPU::getCurrentThreadId();

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
		TEMP_FLUSH_RENDER_COMMAND();
#undef BailIfFalse

		readConfigFile();

		// #todo: Where to put this
		registerExec("profile_gpu", [](const std::string& command) {
			gEngine->dumpGPUProfile();
		});
		registerExec("stat", [](const std::string& command) {
			gEngine->toggleFrameStat();
		});

		LOG(LogInfo, "=== PATHOS has been initialized ===");
		LOG(LogInfo, "");

		return true;
	}

	bool Engine::destroy() {
		LOG(LogInfo, "");
		LOG(LogInfo, "=== Destroy PATHOS ===");

		// #todo: Destroy subsystems managed on main thread.

		LOG(LogInfo, "=== PATHOS has been destroyed ===");
		LOG(LogInfo, "");

		return true;
	}

	bool Engine::initializeMainWindow(int argc, char** argv)
	{
		GUIWindowCreateParams createParams;
		createParams.argc           = argc;
		createParams.argv           = argv;

		createParams.width          = conf.windowWidth;
		createParams.height         = conf.windowHeight;
		createParams.fullscreen     = conf.fullscreen;
		createParams.title          = conf.title;

		createParams.glMajorVersion = REQUIRED_GL_MAJOR_VERSION;
		createParams.glMinorVersion = REQUIRED_GL_MINOR_VERSION;
		createParams.glDebugContext = GL_DEBUG_CONTEXT;

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

		mainWindow = std::make_unique<GUIWindow>();
		mainWindow->create(createParams);

		LOG(LogInfo, "Initialize the main window");
		return true;
	}

	bool Engine::initializeInput()
	{
		inputSystem = std::make_unique<InputSystem>();

		return true;
	}

	bool Engine::initializeAssetStreamer()
	{
		assetStreamer = std::make_unique<AssetStreamer>();

		return true;
	}

	bool Engine::initializeImageLibrary()
	{
		pathos::initializeImageLibrary();
		return true;
	}

	bool Engine::initializeFontSystem(RenderCommandList& cmdList)
	{
		if (FontManager::get().init() == false) {
			LOG(LogError, "[ERROR] Failed to initialize font manager");
			return false;
		}
		FontManager::get().registerFont(cmdList, "default", "resources/fonts/consola.ttf", 28);
		FontManager::get().registerFont(cmdList, "hangul", "resources/fonts/BMJUA.ttf", 28);    // http://font.woowahan.com/jua/
		LOG(LogInfo, "Initialize font subsystem");

		return true;
	}

	bool Engine::initializeConsole()
	{
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
		stopwatch_gameThread.start();
		assetStreamer->initialize(conf.numWorkersForAssetStreamer);
		renderThread->markMainLoopStarted();
		mainWindow->startMainLoop();
	}

	void Engine::stop() {
		mainWindow->stopMainLoop();

		assetStreamer->destroy();
		pathos::destroyImageLibrary();

		renderThread->terminate();

		bool engineDestroyed = destroy();
		CHECKF(engineDestroyed, "Failed to destroy the engine properly !!!");
	}

	// For scene capture
	void Engine::pushSceneProxy(SceneProxy* newSceneProxy) {
		renderThread->pushSceneProxy(newSceneProxy);
	}

	void Engine::updateMainWindow_renderThread() {
		mainWindow->updateWindow_renderThread();
	}

	void Engine::updateGPUQuery_renderThread(
		float inElapsedRenderThread,
		float inElapsedGpu,
		const std::vector<std::string>& inGpuCounterNames,
		const std::vector<float>& inGpuCounterTimes)
	{
		std::lock_guard<std::mutex> lockGuard(gpuQueryMutex);

		elapsed_renderThread = inElapsedRenderThread;
		elapsed_gpu = inElapsedGpu;
		lastGpuCounterNames = inGpuCounterNames;
		lastGpuCounterTimes = inGpuCounterTimes;
	}

	// Read config line by line and add to the console window.
	void Engine::readConfigFile()
	{
		std::string configPath = ResourceFinder::get().find("EngineConfig.ini");
		if (configPath.size() > 0) {
			LOG(LogInfo, "Read config file: %s", configPath.c_str());

			std::fstream file(configPath);
			std::string line;
			while (std::getline(file, line)) {
				// Lines that are empty or start with # are comments
				if (line.size() == 0 || line[0] == '#') {
					continue;
				}
				gConsole->addLine(line.c_str(), false);
			}
		}
	}

	void Engine::registerExec(const char* command, ExecProc proc)
	{
		if (execMap.find(command) != execMap.end()) {
			LOG(LogError, "Exec already registered: %s", command);
			return;
		}
		execMap.insert(std::make_pair(command, proc));
	}

	bool Engine::execute(const std::string& command)
	{
		auto ix = command.find(' ');
		std::string header = ix == string::npos ? command : command.substr(0, ix);

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

	// #todo-gpu-counter: Show this in debug GUI
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

		std::fstream fs(filepath, std::fstream::out);
		if (fs.is_open()) {
			uint32 n = (uint32)lastGpuCounterNames.size();
			for (uint32 i = 0; i < n; ++i) {
				fs << lastGpuCounterNames[i] << ": " << lastGpuCounterTimes[i] << " ms" << std::endl;
			}
			fs.close();
		}

		LOG(LogDebug, "Dump GPU profile to: %s", filepath.c_str());
		gConsole->addLine("Dump GPU profile to:", false);
		gConsole->addLine(filepath.c_str(), false);
	}

	void Engine::setWorld(World* inWorld)
	{
		if (currentWorld != nullptr) {
			currentWorld->destroy();
			delete currentWorld;
		}

		currentWorld = inWorld;
		currentWorld->initialize();
	}

	void Engine::updateScreenSize(int32 inScreenWidth, int32 inScreenHeight) {
		conf.windowWidth = inScreenWidth;
		conf.windowHeight = inScreenHeight;
	}

	void Engine::tick()
	{
		CpuProfiler::getInstance().beginCheckpoint(frameCounter_gameThread);

		// Start render thread with prev frame's scene proxy
		const uint32 frameNumber_renderThread = frameCounter_gameThread;
		renderThread->beginFrame(frameNumber_renderThread);

		// Start world tick
		{
			SCOPED_CPU_COUNTER(WorldTick);

			float deltaSeconds = stopwatch_gameThread.stop();

			// #todo-fps: It only controls the game thread. What about the render thread + GPU?
			if (maxFPS.getValue() > 0 && deltaSeconds < 1.0f / maxFPS.getValue()) {
				elapsed_gameThread = stopwatch_gameThread.stop() * 1000.0f;
				return;
			}

			stopwatch_gameThread.start();

			//
			// Process input
			//
			inputSystem->tick();

			{
				SCOPED_CPU_COUNTER(FlushLoadedAssets);
				gEngine->getAssetStreamer()->flushLoadedAssets();
			}

			//
			// Game tick
			//
			if (currentWorld != nullptr) {
				SCOPED_CPU_COUNTER(CreateRenderProxy);

				const float ar = (float)conf.windowWidth / conf.windowHeight;
				currentWorld->getCamera().getLens().setAspectRatio(ar);

				currentWorld->tick(deltaSeconds);

				// #todo-renderthread: Stupid condition (:p) to prevent scene proxies being queued too much,
				// which makes you feel like there is input lag.
				if (renderThread->mainSceneInSceneProxyQueue() == false) {
					SceneProxy* sceneProxy = currentWorld->getScene().createRenderProxy(
						SceneProxySource::MainScene,
						frameCounter_gameThread,
						currentWorld->getCamera());
					CHECK(sceneProxy != nullptr);

					renderThread->pushSceneProxy(sceneProxy);
				}
			}
		}

		// #todo-cpu: Use frameCounter as a checkpoint
		CpuProfiler::getInstance().finishCheckpoint();

		elapsed_gameThread = stopwatch_gameThread.stop() * 1000.0f;
		stopwatch_gameThread.start();
		if (frameCounter_gameThread == (uint32)(-1)) {
			frameCounter_gameThread = 1;
		} else {
			frameCounter_gameThread += 1;
		}

		// Wait for render thread to finish
		renderThread->endFrame(frameNumber_renderThread);
	}

	/////////////////////////////////////////////////////////////////////
	// GUI callback functions
	/////////////////////////////////////////////////////////////////////

	void Engine::onCloseWindow()
	{
		gEngine->stop();
	}

	void Engine::onIdle()
	{
		// #todo-renderthread: No engine loop work in idle callback.
		//gEngine->tick();
	}

	// #todo-renderthread: This might be not called when the window is minimized.
	//                     How to tick the world, but no draw when minimized?
	void Engine::onMainWindowDisplay() {
		gEngine->tick();
	}

	void Engine::onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY) {
		if (ascii == 0x60) {
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
		gEngine->updateScreenSize(newWidth, newHeight);
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
		gEngine->inputSystem->processButtonDown(mouseInput);
	}

	void Engine::onMouseUp(InputConstants mouseInput, int32 mouseX, int32 mouseY) {
		gEngine->inputSystem->processButtonUp(mouseInput);
	}

}
