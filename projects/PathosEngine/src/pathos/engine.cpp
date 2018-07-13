#include "pathos/engine.h"

#include "GL/gl_core_4_3.h"
//#include "GL/wgl_core.h"
#include "GL/freeglut.h"
#include <assert.h>
#include <iostream>

#include "FreeImage.h" // subsystem: image file loader
#include "pathos/text/font_mgr.h" // subsystem: font manager

namespace pathos {

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

	EngineConfig Engine::conf;
	const std::string Engine::version = "0.1.0";
	const EngineConfig& Engine::getConfig() { return Engine::conf; }
	bool Engine::keymap[256] = { false };
#if PATHOS_MULTI_THREAD_SUPPORT
	HGLRC Engine::mainContext = nullptr;
	HDC Engine::hdc = nullptr;
#endif

	bool Engine::init(int* argcp, char** argv, const EngineConfig& config) {
		std::cout << "=== Initialize pathos engine ===" << '\n';
		std::cout << "- Engine version: " << Engine::version << '\n';

		Engine::conf = config;

		// freeglut
		glutInitErrorFunc(onGlutError);
		glutInitWarningFunc(onGlutWarning);
		glutInit(argcp, argv);
		glutInitContextVersion(4, 3);
		glutInitContextProfile(GLUT_CORE_PROFILE);
#if GL_DEBUG_CONTEXT
		glutInitContextFlags(GLUT_DEBUG);
#endif
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
		glutInitWindowSize(conf.width, conf.height);
		static int wid = glutCreateWindow(conf.title);
		std::cout << "- Main window has been created" << std::endl;

		// GL loader
		if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
		{
			std::cerr << "[ERROR] Failed to initialize GL" << std::endl;
			exit(1);
		}
		std::cout << "- GL version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "- GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

		// multi-threading
#if PATHOS_MULTI_THREAD_SUPPORT
		hdc = wglGetCurrentDC();
		mainContext = wglGetCurrentContext();
#endif

		// FreeImage
		FreeImage_Initialise();
		std::cout << "- FreeImage has been initialized" << std::endl;

		// font manager
		if (FontManager::init() == false) {
			std::cerr << "[ERROR] Failed to initialize font manager" << std::endl;
			return false;
		}
		std::cout << "- Font system has been initialized" << std::endl;
		FontManager::loadFont("default", "../../resources/fonts/consola.ttf", 28);
		// 배달의 민족 주아체 [http://font.woowahan.com/jua/]
		FontManager::loadFont("hangul", "../../resources/fonts/BMJUA.ttf", 28);

		// callbacks
		glutIdleFunc(Engine::idle);
		glutDisplayFunc(Engine::display);
		glutReshapeFunc(Engine::reshape);
		glutKeyboardFunc(Engine::keyDown);
		glutKeyboardUpFunc(Engine::keyUp);
		//glutSpecialFunc(Engine::specialKeyboardCallback);
		//glutSpecialUpFunc(Engine::specialKeyboardUpCallback);

		// render state
		glClearColor(0, 0, 0, 0);
		glClearDepth(1.0f);
		glClearStencil(0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		//glEnable(GL_MULTISAMPLE);
		std::cout << "- GL states have been initialized" << std::endl;

		return true;
	}

	/**
	* start the main loop.
	*/
	void Engine::start() {
		std::cout << "- Start the main loop" << std::endl;
		glutMainLoop();
	}

	/**
	* stop the main loop.
	*/
	void Engine::stop() {
		glutLeaveMainLoop();
		std::cout << "- Stop the main loop" << std::endl;
	}

	void Engine::render() {
		auto callback = Engine::conf.render;
		if (callback != nullptr) callback();
	}

	/////////////////////////////////////////////////////////////////////
	// glut callback functions
	/////////////////////////////////////////////////////////////////////

	void Engine::display() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		Engine::render();
		glutSwapBuffers();
		glutPostRedisplay();
	}

	void Engine::idle() { Engine::render(); }

	void Engine::keyDown(unsigned char ascii, int x, int y) {
		if (Engine::keymap[ascii] == false) {
			Engine::keymap[ascii] = true;
			auto press_callback = Engine::conf.keyPress;
			if (press_callback != nullptr) press_callback(ascii);
		}
		auto callback = Engine::conf.keyDown;
		if (callback != nullptr) callback(ascii, x, y);
	}
	void Engine::keyUp(unsigned char ascii, int x, int y) {
		Engine::keymap[ascii] = false;
		auto callback = Engine::conf.keyUp;
		if (callback != nullptr) callback(ascii, x, y);
	}

	void Engine::reshape(int w, int h) { glViewport(0, 0, w, h); }

#if PATHOS_MULTI_THREAD_SUPPORT
	HGLRC Engine::createContext() {
		HDC hdc = wglGetCurrentDC();
		HGLRC context = wglCreateContext(hdc);
		return context;
	}

	void Engine::deleteContext(HGLRC context) {
		wglDeleteContext(context);
	}
#endif
	
}