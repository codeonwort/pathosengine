#pragma once

// Build configurations
#define PATHOS_MULTI_THREAD_SUPPORT 0
#define GL_DEBUG_CONTEXT 0

#if PATHOS_MULTI_THREAD_SUPPORT
#include "GL/gl_core_4_3.h"
#include "GL/freeglut.h"
#include "GL/wgl_core.h"
#endif

#include <string>

namespace pathos {

	/**
	* Only used for Engine::init()
	*/
	struct EngineConfig {
		int	width;								/** window width. */
		int height;								/** window height. */
		const char* title = "pathos engine";	/** window title. */
		void(*render)() = nullptr;
		void(*keyDown)(unsigned char, int, int) = nullptr;
		void(*keyUp)(unsigned char, int, int) = nullptr;
		void(*keyPress)(unsigned char) = nullptr;
	};

	class Engine final {

	private:
		static EngineConfig conf;
		static void render();
		static bool keymap[256];
#if PATHOS_MULTI_THREAD_SUPPORT
		static HDC hdc;
		static HGLRC mainContext;
#endif

	public:
		static const std::string version;
		static bool init(int* argcp, char** argv, const EngineConfig& conf); /** initialize the engine. */
		static void start(); /** start the main loop. */
		static void stop(); /** stop the main loop. */
		static const EngineConfig& getConfig();
		inline static bool isDown(unsigned char ascii) { return keymap[ascii]; }

		// glut event listeners //
		static void idle();
		static void display();
		static void reshape(int w, int h);
		static void keyDown(unsigned char ascii, int x, int y);
		static void keyUp(unsigned char ascii, int x, int y);

#if PATHOS_MULTI_THREAD_SUPPORT
		static HGLRC createContext();
		static void deleteContext(HGLRC context);
		static inline HDC getHDC() { return hdc; }
#endif
	};

}