#include "pathos/engine.h"
#include "gl_core.h"

#include "GL/freeglut.h"          // subsystem: window
#include "FreeImage.h"            // subsystem: image file loader
#include "pathos/text/font_mgr.h" // subsystem: font manager

#include <assert.h>
#include <iostream>

#define GL_DEBUG_CONTEXT  0
#define GL_ERROR_CALLBACK 1

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

	EngineConfig Engine::conf;
	const std::string Engine::version = "0.1.0";
	const EngineConfig& Engine::getConfig() { return Engine::conf; }
	bool Engine::keymap[256] = { false };

	bool Engine::init(int* argcp, char** argv, const EngineConfig& config) {
		std::cout << "=== Initialize pathos engine ===" << '\n';
		std::cout << "- Engine version: " << Engine::version << '\n';

		Engine::conf = config;

		// freeglut
		glutInitErrorFunc(onGlutError);
		glutInitWarningFunc(onGlutWarning);
		glutInit(argcp, argv);
		glutInitContextVersion(4, 6);
		glutInitContextProfile(GLUT_CORE_PROFILE);
#if GL_DEBUG_CONTEXT
		glutInitContextFlags(GLUT_DEBUG);
#endif
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
		glutInitWindowSize(conf.width, conf.height);
		static int wid = glutCreateWindow(conf.title);
		std::cout << "- Main window has been created" << std::endl;

		// gl3w
		if (gl3wInit()) {
			std::cerr << "failed to initialize GL3W" << std::endl;
			return false;
		}
		if (!gl3wIsSupported(4, 6)) {
			std::cerr << "GL 4.6 is not supported" << std::endl;
			return false;
		}

		std::cout << "- GL version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "- GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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
		// [http://font.woowahan.com/jua/]
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

#if GL_ERROR_CALLBACK
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(glErrorCallback, 0);
#endif
		
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
