#pragma once

#include "badger/types/int_types.h"
#include "badger/system/stopwatch.h"

#include <map>
#include <memory>
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
			, fullscreen(false)
			, title("pathos engine")
			, rendererType(ERendererType::Deferred)
		{
		}

		int32 windowWidth;
		int32 windowHeight;
		bool fullscreen;
		const char* title;

		ERendererType rendererType;

		void(*tick)(float deltaSeconds)     = nullptr;
		void(*render)()                     = nullptr;
		void(*keyDown)(uint8, int32, int32) = nullptr;
		void(*keyUp)(uint8, int32, int32)   = nullptr;
		void(*keyPress)(uint8)              = nullptr;
	};

	class Engine final {

		using ExecProc = std::function<void(const std::string&)>;

	public:
		static bool init(int argc, char** argv, const EngineConfig& conf);

	// Public API
	public:
		void start();
		void stop();

		void registerExec(const char* command, ExecProc proc);
		bool execute(const std::string& command);

		void setWorld(Scene* inScene, Camera* inCamera);

		const EngineConfig& getConfig() const { return conf; }

		// Estimated time of rendering work
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

		bool initialize(int argcp, char** argv, const EngineConfig& conf);

		// #todo-glut: Abstract window system - don't use glut directly
		bool initializeMainWindow(int argcp, char** argv);
		bool initializeOpenGL();
		bool initializeThirdParty();
		bool initializeConsole();
		bool initializeRenderer();

		// glut event listeners //
		static void onIdle();
		static void onMainWindowDisplay();
		static void onMainWindowReshape(int32 newWidth, int32 newHeight);
		static void onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY);
		static void onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY);

	private:
		void tick();
		void render();

	// Game thread
	private:
		EngineConfig conf;
		Stopwatch stopwatch_gameThread;

		Scene* scene;
		Camera* camera;

		std::map<std::string, ExecProc> execMap;

		bool keymap[256] = { false, };

		std::unique_ptr<class GUIWindow> mainWindow;

	// Render thread
	private:
		class OpenGLDevice* render_device;
		Renderer* renderer;

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
