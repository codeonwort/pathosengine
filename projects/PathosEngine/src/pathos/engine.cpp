#include "engine.h"
#include "engine_version.h"
#include "console.h"
#include "render/render_device.h"
#include "render/render_forward.h"
#include "render/render_deferred.h"
#include "util/log.h"
#include "util/resource_finder.h"
#include "util/renderdoc_integration.h"

#include "GL/freeglut.h"          // subsystem: window
#include "FreeImage.h"            // subsystem: image file loader
#include "pathos/text/font_mgr.h" // subsystem: font manager

#include <algorithm>
#include <assert.h>

#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "FreeImage.lib")

static void onGlutError(const char *fmt, va_list ap) {
	fprintf(stderr, "onGlutError:");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");

	/* deInitialize the freeglut state */
	fprintf(stderr, "onGlutError: Calling glutExit()\n");
	glutExit();

	exit(1);
}

static void onGlutWarning(const char *fmt, va_list ap) {
	fprintf(stderr, "onGlutWarning:");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");

	/* deInitialize the freeglut state */
	fprintf(stderr, "onGlutWarning: Calling glutExit()\n");
	glutExit();

	exit(1);
}

namespace pathos {

	Engine*        gEngine  = nullptr;
	ConsoleWindow* gConsole = nullptr;

	//////////////////////////////////////////////////////////////////////////
	// static
	bool Engine::init(int* argcp, char** argv, const EngineConfig& config) {
		if (gEngine) {
			LOG(LogWarning, "The engine is already initialized");
			return false;
		}

		gEngine = new Engine;
		gEngine->initialize(argcp, argv, config);

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

	bool Engine::initialize(int* argcp, char** argv, const EngineConfig& config)
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
		BailIfFalse( initializeGlut(argcp, argv) );
		BailIfFalse( initializeOpenGL()          );
		BailIfFalse( initializeThirdParty()      );
		BailIfFalse( initializeConsole()         );
		BailIfFalse( initializeRenderer()        );
#undef BailIfFalse

		LOG(LogInfo, "=== PATHOS has been initialized ===");
		LOG(LogInfo, "");

		return true;
	}

	bool Engine::initializeGlut(int* argcp, char** argv)
	{
		glutInitErrorFunc(onGlutError);
		glutInitWarningFunc(onGlutWarning);
		glutInit(argcp, argv);
		glutInitContextVersion(REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION);
		glutInitContextProfile(GLUT_CORE_PROFILE);
#if GL_DEBUG_CONTEXT
		glutInitContextFlags(GLUT_DEBUG);
#endif
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
		glutInitWindowSize(conf.windowWidth, conf.windowHeight);
		glutCreateWindow(conf.title);

		// callbacks
		glutIdleFunc(Engine::onGlutIdle);
		glutDisplayFunc(Engine::onGlutDisplay);
		glutReshapeFunc(Engine::onGlutReshape);
		glutKeyboardFunc(Engine::onGlutKeyDown);
		glutKeyboardUpFunc(Engine::onGlutKeyUp);
		//glutSpecialFunc(Engine::specialKeyboardCallback);
		//glutSpecialUpFunc(Engine::specialKeyboardUpCallback);

		LOG(LogInfo, "Main window has been created");
		return true;
	}

	bool Engine::initializeOpenGL()
	{
		render_device = new OpenGLDevice;
		bool validDevice = render_device->initialize();

		glGenQueries(1, &timer_query);
		CHECK(timer_query != 0);

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
		LOG(LogInfo, "Start the main loop");
		glutMainLoop();
	}

	void Engine::stop() {
		glutLeaveMainLoop();
		LOG(LogInfo, "Stop the main loop");
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
	// glut callback functions
	/////////////////////////////////////////////////////////////////////

	void Engine::onGlutIdle()
	{
		gEngine->tick();
	}

	void Engine::onGlutDisplay() {
		gEngine->render();
		glutSwapBuffers();
		glutPostRedisplay();
	}

	void Engine::onGlutKeyDown(unsigned char ascii, int x, int y) {
		if (gEngine->keymap[ascii] == false) {
			gEngine->keymap[ascii] = true;
			auto press_callback = gEngine->conf.keyPress;
			if (press_callback != nullptr) press_callback(ascii);
		}

		// backtick
		if (ascii == 0x60) {
			gConsole->toggle();
		} else if (gConsole->isVisible()) {
			gConsole->onKeyPress(ascii);
		} else {
			auto callback = gEngine->conf.keyDown;
			if (callback != nullptr) callback(ascii, x, y);
		}
	}

	void Engine::onGlutKeyUp(unsigned char ascii, int x, int y) {
		gEngine->keymap[ascii] = false;
		auto callback = gEngine->conf.keyUp;
		if (callback != nullptr) callback(ascii, x, y);
	}

	void Engine::onGlutReshape(int w, int h) {
		// Will be handled by renderer
		// glViewport(0, 0, w, h);
	}

}
