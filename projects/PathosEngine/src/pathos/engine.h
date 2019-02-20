#pragma once

#include <string>
#include "gl_core.h"

namespace pathos {

	// @see Engine::init
	struct EngineConfig {
		int	windowWidth;						/** window width. */
		int windowHeight;						/** window height. */
		const char* title = "pathos engine";	/** window title. */

		void(*tick)()                           = nullptr;
		void(*render)()                         = nullptr;
		void(*keyDown)(unsigned char, int, int) = nullptr;
		void(*keyUp)(unsigned char, int, int)   = nullptr;
		void(*keyPress)(unsigned char)          = nullptr;
	};

	class Engine final {

	public:
		static const std::string version;

		static bool init(int* argcp, char** argv, const EngineConfig& conf);

	private:
		Engine();
		~Engine();
		Engine(const Engine& other) = delete;
		Engine& operator=(const Engine& other) = delete;

		bool initialize(int* argcp, char** argv, const EngineConfig& conf);
		bool initializeGlut(int* argcp, char** argv);
		bool initializeOpenGL();
		bool initializeThirdParty();
		bool initializeConsole();

		// glut event listeners //
		static void onGlutIdle();
		static void onGlutDisplay();
		static void onGlutReshape(int w, int h);
		static void onGlutKeyDown(unsigned char ascii, int x, int y);
		static void onGlutKeyUp(unsigned char ascii, int x, int y);

	public:
		void start();
		void stop();
		const EngineConfig& getConfig() const { return conf; }

		inline float getMilliseconds() const { return elapsed_ms; }
		inline bool isDown(unsigned char ascii) { return keymap[ascii]; }

	private:
		void tick();
		void render();

		EngineConfig conf;
		bool keymap[256] = { false, };

		GLuint timer_query;
		float elapsed_ms;

	};

	extern Engine* gEngine;
	extern class ConsoleWindow* gConsole;

}
