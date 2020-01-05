#include "engine.h"
#include "engine_version.h"
#include "console.h"
#include "render/render_device.h"
#include "render/render_forward.h"
#include "render/render_deferred.h"
#include "util/log.h"
#include "util/resource_finder.h"
#include "util/renderdoc_integration.h"

#include "FreeImage.h"             // subsystem: image file loader
#include "pathos/text/font_mgr.h"  // subsystem: font manager
#include "pathos/gui/gui_window.h" // subsystem: gui

#include <algorithm>

#pragma comment(lib, "FreeImage.lib")

namespace pathos {

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
		gEngine->initialize(argc, argv, config);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	Engine::Engine()
		: renderer(nullptr)
		, scene(nullptr)
		, camera(nullptr)
		, timer_query(0)
		, elapsed_ms(0)
	{
	}

	Engine::~Engine()
	{
	}

	bool Engine::initialize(int argc, char** argv, const EngineConfig& config)
	{
		conf = config;

		LOG(LogInfo, "");
		LOG(LogInfo, "===      Initialize PATHOS      ===");
		LOG(LogInfo, "Engine version: %d.%d.%d", PATHOS_MAJOR_VERSION, PATHOS_MINOR_VERSION, PATHOS_PATCH_VERSION);

		ResourceFinder::get().add("../");
		ResourceFinder::get().add("../../");
		ResourceFinder::get().add("../../shaders/");
		ResourceFinder::get().add("../../resources/");

		RenderDocIntegration::get().findInjectedDLL();

#define BailIfFalse(x) if(!(x)) { return false; }
		BailIfFalse( initializeMainWindow(argc, argv) );
		BailIfFalse( initializeOpenGL()               );
		BailIfFalse( initializeThirdParty()           );
		BailIfFalse( initializeConsole()              );
		BailIfFalse( initializeRenderer()             );
#undef BailIfFalse

		LOG(LogInfo, "=== PATHOS has been initialized ===");
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

		createParams.onIdle    = Engine::onIdle;
		createParams.onDisplay = Engine::onMainWindowDisplay;
		createParams.onKeyDown = Engine::onKeyDown;
		createParams.onKeyUp   = Engine::onKeyUp;
		createParams.onReshape = Engine::onMainWindowReshape;

		mainWindow = std::make_unique<GUIWindow>();
		mainWindow->create(createParams);

		LOG(LogInfo, "Main window has been created");
		return true;
	}

	bool Engine::initializeOpenGL()
	{
		render_device = new OpenGLDevice;
		bool validDevice = render_device->initialize();

		glGenQueries(1, &timer_query);
		CHECK(timer_query != 0);

		{
			GLuint systemTextures[4];
			glCreateTextures(GL_TEXTURE_2D, 4, systemTextures);

			texture2D_black = systemTextures[0];
			texture2D_white = systemTextures[1];
			texture2D_grey  = systemTextures[2];
			texture2D_blue  = systemTextures[3];

			glTextureStorage2D(texture2D_black, 1, GL_RGBA8, 1, 1);
			glTextureStorage2D(texture2D_white, 1, GL_RGBA8, 1, 1);
			glTextureStorage2D(texture2D_grey,  1, GL_RGBA8, 1, 1);
			glTextureStorage2D(texture2D_blue,  1, GL_RGBA8, 1, 1);

			GLubyte black[4] = { 0, 0, 0, 0 };
			GLubyte white[4] = { 0xff, 0xff, 0xff, 0xff };
			GLubyte grey[4]  = { 0x7f, 0x7f, 0x7f, 0x7f };
			GLubyte blue[4]  = { 0x00, 0x00, 0xff, 0xff };

			glClearTexImage(texture2D_black, 0, GL_RGBA, GL_UNSIGNED_BYTE, black);
			glClearTexImage(texture2D_white, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
			glClearTexImage(texture2D_grey,  0, GL_RGBA, GL_UNSIGNED_BYTE, grey);
			glClearTexImage(texture2D_blue,  0, GL_RGBA, GL_UNSIGNED_BYTE, blue);
		}

		return validDevice;
	}

	bool Engine::initializeThirdParty()
	{
		// FreeImage
		FreeImage_Initialise();
		LOG(LogInfo, "FreeImage has been initialized");

		// font manager
		if (FontManager::init() == false) {
			LOG(LogError, "[ERROR] Failed to initialize font manager");
			return false;
		}
		FontManager::loadFont("default", "../../resources/fonts/consola.ttf", 28);
		FontManager::loadFont("hangul", "../../resources/fonts/BMJUA.ttf", 28);    // [http://font.woowahan.com/jua/]
		LOG(LogInfo, "Font system has been initialized");

		return true;
	}

	bool Engine::initializeConsole()
	{
		gConsole = new ConsoleWindow;
		if (gConsole->initialize(conf.windowWidth, std::min(conf.windowHeight, 400)) == false) {
			LOG(LogError, "Failed to initialize console window");
			return false;
		}
		gConsole->addLine(L"Built-in debug console. Press ` to toggle.");

		LOG(LogInfo, "Debug console has been created");
		return true;
	}

	bool Engine::initializeRenderer()
	{
		switch (conf.rendererType) {
		case ERendererType::Forward:
			renderer = new ForwardRenderer;
			break;

		case ERendererType::Deferred:
			renderer = new DeferredRenderer(conf.windowWidth, conf.windowHeight);
			break;
		}

		if(renderer) {
			renderer->initializeResources(render_device->getImmediateCommandList());
			render_device->getImmediateCommandList().flushAllCommands();
		}

		return renderer != nullptr;
	}

	void Engine::start() {
		mainWindow->startMainLoop();
	}

	void Engine::stop() {
		mainWindow->stopMainLoop();
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

	void Engine::setWorld(Scene* inScene, Camera* inCamera)
	{
		scene = inScene;
		camera = inCamera;
	}

	void Engine::tick()
	{
		// #todo-tick: add option to limit fps
		auto callback = Engine::conf.tick;
		if (callback != nullptr) {
			callback();
		}
	}

	void Engine::render() {
		GLuint64 elapsed_ns;
		glBeginQuery(GL_TIME_ELAPSED, timer_query);

		RenderCommandList& immediateContext = gRenderDevice->getImmediateCommandList();

		// #todo-cmd-list: deferred command lists here

		// Renderer adds more immediate commands
		if (renderer && scene && camera) {
			renderer->render(immediateContext, scene, camera);
			immediateContext.flushAllCommands();
		}

		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(timer_query, GL_QUERY_RESULT, &elapsed_ns);
		elapsed_ms = (float)elapsed_ns / 1000000.0f;

		// #todo-console: Fix
		if (gConsole) {
			gConsole->renderConsoleWindow(immediateContext);
			immediateContext.flushAllCommands();
		}
	}

	/////////////////////////////////////////////////////////////////////
	// GUI callback functions
	/////////////////////////////////////////////////////////////////////

	void Engine::onIdle()
	{
		gEngine->tick();
	}

	void Engine::onMainWindowDisplay() {
		gEngine->render();
	}

	void Engine::onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY) {
		if (gEngine->keymap[ascii] == false) {
			gEngine->keymap[ascii] = true;

			auto press_callback = gEngine->conf.keyPress;
			if (press_callback != nullptr) {
				press_callback(ascii);
			}
		}

		// backtick
		if (ascii == 0x60) {
			gConsole->toggle();
		} else if (gConsole->isVisible()) {
			gConsole->onKeyPress(ascii);
		} else {
			auto callback = gEngine->conf.keyDown;
			if (callback != nullptr) {
				callback(ascii, mouseX, mouseY);
			}
		}
	}

	void Engine::onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY) {
		gEngine->keymap[ascii] = false;

		auto callback = gEngine->conf.keyUp;
		if (callback != nullptr) {
			callback(ascii, mouseX, mouseY);
		}
	}

	void Engine::onMainWindowReshape(int32 newWidth, int32 newHeight) { }

}
