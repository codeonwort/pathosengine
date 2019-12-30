#pragma once

#include "badger/types/int_types.h"

#include <map>
#include <string>
#include <functional>
#include "gl_core.h"

namespace pathos {

	class Renderer;
	class Scene;
	class Camera;

	enum class ERendererType : uint8 {
		Forward,
		Deferred
	};

	// @see Engine::init
	struct EngineConfig {
		EngineConfig()
			: windowWidth(1920)
			, windowHeight(1080)
			, title("pathos engine")
			, rendererType(ERendererType::Forward)
		{
		}

		int windowWidth;
		int windowHeight;
		const char* title;

		ERendererType rendererType;

		void(*tick)()                           = nullptr;
		void(*render)()                         = nullptr;
		void(*keyDown)(unsigned char, int, int) = nullptr;
		void(*keyUp)(unsigned char, int, int)   = nullptr;
		void(*keyPress)(unsigned char)          = nullptr;
	};

	class Engine final {

		using ExecProc = std::function<void(const std::string&)>;

	public:
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

		// #todo-input: Make an input manager
		inline bool isDown(unsigned char ascii) { return keymap[ascii]; }

		inline GLuint getSystemTexture2DBlack() const { return texture2D_black; }
		inline GLuint getSystemTexture2DWhite() const { return texture2D_white; }
		inline GLuint getSystemTexture2DGrey()  const { return texture2D_grey;  }
		inline GLuint getSystemTexture2DBlue()  const { return texture2D_blue;  }

	private:
		Engine();
		~Engine();

		Engine(const Engine&)            = delete;
		Engine& operator=(const Engine&) = delete;

		bool initialize(int* argcp, char** argv, const EngineConfig& conf);

		// #todo-glut: Abstract window system - don't use glut directly
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

		class OpenGLDevice* render_device;
		Renderer* renderer;
		Scene* scene;
		Camera* camera;

		std::map<std::string, ExecProc> execMap;

		bool keymap[256] = { false, };

		GLuint timer_query;
		float elapsed_ms;

		// System textures
		GLuint texture2D_black = 0;
		GLuint texture2D_white = 0;
		GLuint texture2D_grey  = 0;
		GLuint texture2D_blue  = 0;

	};

	extern Engine* gEngine;
	extern class ConsoleWindow* gConsole;

}
