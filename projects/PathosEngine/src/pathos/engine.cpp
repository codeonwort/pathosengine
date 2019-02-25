#include "engine.h"
#include "console.h"
#include "render/render_forward.h"
#include "render/render_deferred.h"
#include "util/log.h"
#include "util/resource_finder.h"
#include "util/renderdoc_integration.h"

#include "GL/freeglut.h"          // subsystem: window
#include "FreeImage.h"            // subsystem: image file loader
#include "pathos/text/font_mgr.h" // subsystem: font manager

#include <assert.h>
#include <iostream>
#include <algorithm>

#define GL_DEBUG_CONTEXT  0
#define GL_ERROR_CALLBACK 1

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

#if GL_ERROR_CALLBACK
void glErrorCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		fprintf_s(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
		__debugbreak();
	}
}
#endif

namespace pathos {

	Engine*        gEngine  = nullptr;
	ConsoleWindow* gConsole = nullptr;

	const std::string Engine::version = "0.2.0";

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
		LOG(LogInfo, "Engine version: %s", Engine::version.data());

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
		glutInitContextVersion(4, 6);
		glutInitContextProfile(GLUT_CORE_PROFILE);
#if GL_DEBUG_CONTEXT
		glutInitContextFlags(GLUT_DEBUG);
#endif
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
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
		if (gl3wInit()) {
			LOG(LogError, "Failed to initialize GL3W");
			return false;
		}
		if (!gl3wIsSupported(4, 6)) {
			LOG(LogError, "GL 4.6 is not supported");
			return false;
		}

		// render state
		glClearColor(0, 0, 0, 0);
		glClearDepth(1.0f);
		glClearStencil(0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

#if GL_ERROR_CALLBACK
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(glErrorCallback, 0);
#endif

		glGenQueries(1, &timer_query);
		assert(timer_query != 0);

		LOG(LogInfo, "GL version: %s", glGetString(GL_VERSION));
		LOG(LogInfo, "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
		return true;
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
		auto callback = Engine::conf.tick;
		if (callback != nullptr) callback();
	}

	void Engine::render() {
		GLuint64 elapsed_ns;
		glBeginQuery(GL_TIME_ELAPSED, timer_query);

		if (renderer && scene && camera) {
			renderer->render(scene, camera);
		}
		
// 		if (conf.render != nullptr) {
// 			conf.render();
// 		}

		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectui64v(timer_query, GL_QUERY_RESULT, &elapsed_ns);
		elapsed_ms = (float)elapsed_ns / 1000000.0f;

		if (gConsole) {
			gConsole->render();
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

	void Engine::onGlutReshape(int w, int h) { glViewport(0, 0, w, h); }

}
