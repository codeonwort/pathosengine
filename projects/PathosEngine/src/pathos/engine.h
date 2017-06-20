#pragma once

#include <string>

namespace pathos {

	/**
	* Only used for Engine::init()
	*/
	struct EngineConfig {
		int width;								/** window width. */
		int height;								/** window height. */
		const char* title = "pathos engine";	/** window title. */
		void (*render)() = nullptr;				/** render function */
		void (*keyDown)(unsigned char, int, int) = nullptr;		/** keyDown function */
		void (*keyUp)(unsigned char, int, int) = nullptr;		/** keyUp function */
	};

	class Engine final {
	private:
		static EngineConfig conf;
		static void render();
		static bool keymap[256];
	public:
		static std::string version;
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
	};

}