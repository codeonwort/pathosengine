#include "pathos/engine.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "pathos/text/font_mgr.h" // subsystem: font manager
#include "FreeImage.h" // subsystem: image file loader

#include <iostream>

namespace pathos {

	EngineConfig Engine::conf;
	std::string Engine::version = "1.0.0.0";
	const EngineConfig& Engine::getConfig() { return Engine::conf; }
	bool Engine::keymap[256] = { false };
#ifdef PATHOS_MULTI_THREAD_SUPPORT
	HGLRC Engine::mainContext = nullptr;
	HDC Engine::hdc = nullptr;
#endif

	bool Engine::init(int* argcp, char** argv, const EngineConfig& config) {
		std::cout << "initialize pathos engine..." << std::endl;
		std::cout << "engine version: " << Engine::version << std::endl;

		Engine::conf = config;

		// freeglut
		//glutInitContextProfile(GLUT_CORE_PROFILE);
		//glutInitContextFlags(GLUT_DEBUG);
		glutInit(argcp, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
		glutInitWindowSize(conf.width, conf.height);
		static int wid = glutCreateWindow(conf.title);
		std::cout << "- window created" << std::endl;

		// glew
		if (glewInit() != GLEW_OK) exit(1);
		std::cout << "- glew initialized" << std::endl;

		// multi-threading
#ifdef PATHOS_MULTI_THREAD_SUPPORT
		hdc = wglGetCurrentDC();
		mainContext = wglGetCurrentContext();
#endif

		// FreeImage
		FreeImage_Initialise();
		std::cout << "- freeimage initialized" << std::endl;

		// font manager
		if (FontManager::init() == false) {
			std::cerr << "Failed to initialize the font manager" << std::endl;
			return false;
		}
		FontManager::loadFont("default", "../../resources/fonts/consola.ttf", 28); // consolas
		FontManager::loadFont("hangul", "../../resources/fonts/BMJUA.ttf", 28); // 배달의 민족 주아체 [http://font.woowahan.com/jua/]

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
		glEnable(GL_MULTISAMPLE);
		std::cout << "- gl state initialized" << std::endl;

		return true;
	}

	/**
	* start the main loop.
	*/
	void Engine::start() {
		std::cout << "main loop start" << std::endl;
		glutMainLoop();
	}

	/**
	* stop the main loop.
	*/
	void Engine::stop() {
		glutLeaveMainLoop();
		std::cout << "main loop stop" << std::endl;
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
		Engine::keymap[ascii] = true;
		auto callback = Engine::conf.keyDown;
		if (callback != nullptr) callback(ascii, x, y);
	}
	void Engine::keyUp(unsigned char ascii, int x, int y) {
		Engine::keymap[ascii] = false;
		auto callback = Engine::conf.keyUp;
		if (callback != nullptr) callback(ascii, x, y);
	}

	void Engine::reshape(int w, int h) { glViewport(0, 0, w, h); }

#ifdef PATHOS_MULTI_THREAD_SUPPORT
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