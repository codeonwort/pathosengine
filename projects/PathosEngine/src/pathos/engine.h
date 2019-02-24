#pragma once

#include <map>
#include <string>
#include <functional>
#include "gl_core.h"

namespace pathos {

	class Renderer;
	class Scene;
	class Camera;

	enum class ERendererType : uint8_t {
		Forward,
		Deferred
	};

	// @see Engine::init
	struct EngineConfig {
		int	windowWidth;						// window width
		int windowHeight;						// window height
		const char* title = "pathos engine";	// window title

		ERendererType rendererType = ERendererType::Forward;

		void(*tick)()                           = nullptr;
		void(*render)()                         = nullptr;
		void(*keyDown)(unsigned char, int, int) = nullptr;
		void(*keyUp)(unsigned char, int, int)   = nullptr;
		void(*keyPress)(unsigned char)          = nullptr;
	};

	class Engine final {

		using ExecProc = std::function<void(const std::string&)>;

	public:
		static const std::string version;

		static bool init(int* argcp, char** argv, const EngineConfig& conf);

	// Public API
	public:
		void start();
		void stop();

		void registerExec(const char* command, ExecProc proc);
		bool execute(const std::string& command);

		void setWorld(Scene* inScene, Camera* inCamera);

		const EngineConfig& getConfig() const { return conf; }

		inline float getMilliseconds() const { return elapsed_ms; }
		inline bool isDown(unsigned char ascii) { return keymap[ascii]; }

	// Debug API for development
	public:
		inline Renderer* debug_getRenderer() const { return renderer; }

	private:
		Engine();
		~Engine();

		Engine(const Engine& other)            = delete;
		Engine& operator=(const Engine& other) = delete;

		bool initialize(int* argcp, char** argv, const EngineConfig& conf);

		bool initializeGlut(int* argcp, char** argv);
		bool initializeOpenGL();
		bool initializeThirdParty();
		bool initializeConsole();
		bool initializeRenderer();

		// glut event listeners //
		static void onGlutIdle();
		static void onGlutDisplay();
		static void onGlutReshape(int w, int h);
		static void onGlutKeyDown(unsigned char ascii, int x, int y);
		static void onGlutKeyUp(unsigned char ascii, int x, int y);

	private:
		void tick();
		void render();

		EngineConfig conf;

		Renderer* renderer;
		Scene* scene;
		Camera* camera;

		std::map<std::string, ExecProc> execMap;

		bool keymap[256] = { false, };

		GLuint timer_query;
		float elapsed_ms;

	};

	extern Engine* gEngine;
	extern class ConsoleWindow* gConsole;

}
