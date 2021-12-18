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

	static constexpr uint32 RENDER_PROXY_MEMORY = 32 * 1024 * 1024; // 32 MB

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
		: renderProxyAllocator(RENDER_PROXY_MEMORY)
		, frameCounter_gameThread(0)
		, frameCounter_renderThread(0)
		, elapsed_gameThread(0.0f)
		, elapsed_renderThread(0.0f)
		, currentWorld(nullptr)
		, renderThread(nullptr)
		, render_device(nullptr)
		, renderer(nullptr)
		, renderer2D(nullptr)
		, debugOverlay(nullptr)
		, timer_query(0)
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

		renderThread = new RenderThread;

		ResourceFinder::get().add("../");
		ResourceFinder::get().add("../../");
		ResourceFinder::get().add("../../shaders/");
		ResourceFinder::get().add("../../resources/");

		RenderDocIntegration::get().findInjectedDLL();

#define BailIfFalse(x) if(!(x)) { return false; }
		BailIfFalse( initializeMainWindow(argc, argv) );
		BailIfFalse( initializeInput()                );
		BailIfFalse( initializeAssetStreamer()        );
		BailIfFalse( initializeOpenGL()               );
		BailIfFalse( initializeImageLibrary()         );
		BailIfFalse( initializeFontSystem()           );
		BailIfFalse( initializeOverlayRenderer()      );
		BailIfFalse( initializeConsole()              );
		BailIfFalse( initializeRenderer()             );
#undef BailIfFalse

		CpuProfiler& cpuProfiler = CpuProfiler::getInstance();
		cpuProfiler.initialize();
		cpuProfiler.registerCurrentThread("main thread");

		readConfigFile();

		// #todo: Where to put this
		registerExec("profile_gpu", [](const std::string& command) {
			gEngine->dumpGPUProfile();
		});
		registerExec("stat", [](const std::string& command) {
			gEngine->debugOverlay->toggleFrameStat();
		});

		LOG(LogInfo, "=== PATHOS has been initialized ===");
		LOG(LogInfo, "");

		return true;
	}

	bool Engine::destroy() {
		LOG(LogInfo, "");
		LOG(LogInfo, "=== Destroy PATHOS ===");

		// #todo: Destroy other subsystems, but it's meaningless unless GLUT is dealt with.
#define BailIfFalse(x) if(!(x)) { return false; }
		BailIfFalse(destroyOpenGL());
#undef BailIfFalse

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

	bool Engine::initializeOpenGL()
	{
		render_device = new OpenGLDevice;
		bool validDevice = render_device->initialize();

		ENQUEUE_RENDER_COMMAND([this](RenderCommandList& cmdList) {
			render_device->genQueries(1, &timer_query);
			CHECK(timer_query != 0);

			// Create engine resources
			GLuint systemTextures[4];
			render_device->createTextures(GL_TEXTURE_2D, 4, systemTextures);

			texture2D_black = systemTextures[0];
			texture2D_white = systemTextures[1];
			texture2D_grey = systemTextures[2];
			texture2D_blue = systemTextures[3];

			cmdList.textureStorage2D(texture2D_black, 1, GL_RGBA8, 1, 1);
			cmdList.textureStorage2D(texture2D_white, 1, GL_RGBA8, 1, 1);
			cmdList.textureStorage2D(texture2D_grey, 1, GL_RGBA8, 1, 1);
			cmdList.textureStorage2D(texture2D_blue, 1, GL_RGBA8, 1, 1);

			GLubyte black[4] = { 0, 0, 0, 0 };
			GLubyte white[4] = { 0xff, 0xff, 0xff, 0xff };
			GLubyte grey[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
			GLubyte blue[4] = { 0x00, 0x00, 0xff, 0xff };

			cmdList.clearTexImage(texture2D_black, 0, GL_RGBA, GL_UNSIGNED_BYTE, black);
			cmdList.clearTexImage(texture2D_white, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
			cmdList.clearTexImage(texture2D_grey, 0, GL_RGBA, GL_UNSIGNED_BYTE, grey);
			cmdList.clearTexImage(texture2D_blue, 0, GL_RGBA, GL_UNSIGNED_BYTE, blue);

			cmdList.objectLabel(GL_TEXTURE, texture2D_black, -1, "system texture 2D (black)");
			cmdList.objectLabel(GL_TEXTURE, texture2D_white, -1, "system texture 2D (white)");
			cmdList.objectLabel(GL_TEXTURE, texture2D_grey, -1, "system texture 2D (grey)");
			cmdList.objectLabel(GL_TEXTURE, texture2D_blue, -1, "system texture 2D (blue)");
		});

		FLUSH_RENDER_COMMAND();

		for(GlobalRenderRoutine routine : getGlobalRenderRoutineContainer().initRoutines) {
			routine(render_device);
		}

		ScopedGpuCounter::initializeQueryObjectPool();

		return validDevice;
	}

	bool Engine::initializeImageLibrary()
	{
		pathos::initializeImageLibrary();

		return true;
	}

	bool Engine::initializeFontSystem()
	{
		if (FontManager::get().init() == false) {
			LOG(LogError, "[ERROR] Failed to initialize font manager");
			return false;
		}
		FontManager::get().registerFont("default", "resources/fonts/consola.ttf", 28);
		FontManager::get().registerFont("hangul", "resources/fonts/BMJUA.ttf", 28);    // http://font.woowahan.com/jua/
		LOG(LogInfo, "Initialize font subsystem");

		return true;
	}

	bool Engine::initializeOverlayRenderer()
	{
		renderer2D = new OverlayRenderer;

		debugOverlay = new DebugOverlay(renderer2D);
		debugOverlay->initialize();

		return true;
	}

	bool Engine::initializeConsole()
	{
		gConsole = new ConsoleWindow(renderer2D);
		if (gConsole->initialize(conf.windowWidth, std::min(conf.windowHeight, CONSOLE_WINDOW_MIN_HEIGHT)) == false) {
			LOG(LogError, "Failed to initialize console window");
			return false;
		}
		gConsole->addLine(L"Built-in debug console. Press ` to toggle.");

		LOG(LogInfo, "Initialize the debug console");
		return true;
	}

	bool Engine::initializeRenderer()
	{
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
			{
				SceneRenderSettings settings;
				settings.sceneWidth        = conf.windowWidth;
				settings.sceneHeight       = conf.windowHeight;
				settings.enablePostProcess = true;
				renderer->setSceneRenderSettings(settings);
			}
			renderer->initializeResources(render_device->getImmediateCommandList());
			render_device->getImmediateCommandList().flushAllCommands();
		}

		return renderer != nullptr;
	}

	void Engine::start() {
		stopwatch_gameThread.start();
		assetStreamer->initialize(conf.numWorkersForAssetStreamer);
		renderThread->run();
		mainWindow->startMainLoop();
	}

	void Engine::stop() {
		mainWindow->stopMainLoop();

		assetStreamer->destroy();
		pathos::destroyImageLibrary();

		for (GlobalRenderRoutine routine : getGlobalRenderRoutineContainer().destroyRoutines) {
			routine(render_device);
		}

		renderThread->terminate();

		bool engineDestroyed = destroy();
		CHECKF(engineDestroyed, "Failed to destroy the engine properly !!!");
	}

	bool Engine::destroyOpenGL() {
		ScopedGpuCounter::destroyQueryObjectPool();

		return true;
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

	// #todo-gpu-counter: Show this in debug GUI
	void Engine::dumpGPUProfile()
	{
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

	void Engine::tick()
	{
		// Wait for previous frame
		FLUSH_RENDER_COMMAND();

		CpuProfiler::getInstance().beginCheckpoint(frameCounter_gameThread);

		// #todo-renderthread: Render thread shoult start here. It's empty now...
		const uint32 frameNumber_renderThread = frameCounter_gameThread;
		renderThread->beginFrame(frameNumber_renderThread);

		{
			SCOPED_CPU_COUNTER(WorldTick);

			float deltaSeconds = stopwatch_gameThread.stop();

			// #todo-fps: It only controls the game thread. What about the render thread + GPU?
			if (maxFPS.getValue() > 0 && deltaSeconds < 1.0f / maxFPS.getValue()) {
				elapsed_gameThread = stopwatch_gameThread.stop() * 1000.0f;
				if (currentWorld != nullptr) {
					currentWorld->getScene().createRenderProxy();
				}
				return;
			}

			stopwatch_gameThread.start();

			//
			// Process input
			//
			inputSystem->tick();

			//
			// Game tick
			//
			if (currentWorld != nullptr) {
				SCOPED_CPU_COUNTER(CreateRenderProxy);

				currentWorld->tick(deltaSeconds);

				// #todo: More robust way to check if the main window is minimized
				bool windowMinimized = renderProxyAllocator.isClear();
				if (windowMinimized) {
					currentWorld->getScene().createRenderProxy();
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

		// #todo-renderthread: renderThreadMain() should run render() and we just wait for its completion.
		renderThread->endFrame(frameNumber_renderThread);

		//
		// Render tick
		//
		// #todo-renderthread: Currently running them sequentially, but game (N) and render (N-1) should run simultaneously.
		render();
	}

	// All render thread operations are processed here
	void Engine::render() {
		// Render thread should be one frame behind of the game thread
		if (frameCounter_renderThread >= frameCounter_gameThread) {
			return;
		}
		frameCounter_renderThread = frameCounter_gameThread - 1;

		RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();

		GLuint64 elapsed_ns;
		immediateContext.beginQuery(GL_TIME_ELAPSED, timer_query);

		stopwatch_renderThread.start();

		SCOPED_CPU_COUNTER(EngineRender);

		{
			SCOPED_CPU_COUNTER(FlushLoadedAssets);
			assetStreamer->renderThread_flushLoadedAssets();
		}

		{
			currentWorld->getScene().updateDynamicData_renderThread(immediateContext);
		}

		{
			SceneRenderSettings settings;
			settings.sceneWidth            = conf.windowWidth; // #todo: Current window size
			settings.sceneHeight           = conf.windowHeight;
			settings.frameCounter          = frameCounter_renderThread;
			settings.enablePostProcess     = true;
			renderer->setSceneRenderSettings(settings);
		}

		// #todo-cmd-list: deferred command lists here

		// Renderer will add more immediate commands
		if (renderer && currentWorld) {
			SCOPED_CPU_COUNTER(ExecuteRenderer);
			renderer->render(immediateContext, &currentWorld->getScene(), &currentWorld->getCamera());
			immediateContext.flushAllCommands();
		}

		debugOverlay->renderDebugOverlay(immediateContext, conf.windowWidth, conf.windowHeight);
		immediateContext.flushAllCommands();

		if (gConsole) {
			SCOPED_CPU_COUNTER(ExecuteDebugConsole);
			gConsole->renderConsoleWindow(immediateContext);
			immediateContext.flushAllCommands();
		}

		immediateContext.endQuery(GL_TIME_ELAPSED);
		immediateContext.getQueryObjectui64v(timer_query, GL_QUERY_RESULT, &elapsed_ns);
		immediateContext.flushAllCommands();
		elapsed_gpu = (float)elapsed_ns / 1000000.0f;

		// Get GPU profile
		const uint32 numGpuCounters = ScopedGpuCounter::flushQueries(lastGpuCounterNames, lastGpuCounterTimes);

		// Clear various render resources after all rendering is done
		{
			renderProxyAllocator.clear();

			if (currentWorld != nullptr) {
				currentWorld->getScene().clearRenderProxy();
			}

			FontManager::get().onFrameEnd();
		}

		elapsed_renderThread = stopwatch_renderThread.stop() * 1000.0f;
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

	void Engine::onMainWindowReshape(int32 newWidth, int32 newHeight) { }

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
