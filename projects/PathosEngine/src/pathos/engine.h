#pragma once

#include "badger/types/int_types.h"
#include "badger/memory/mem_alloc.h"
#include "badger/system/stopwatch.h"

#include "pathos/input/input_system.h"

#include <map>
#include <list>
#include <memory>
#include <string>
#include <functional>
#include "gl_core.h"

namespace pathos {

	class Renderer;
	class Scene;
	class Camera;
	class InputSystem;
	class AssetStreamer;

	enum class ERendererType : uint8 {
		Forward, // #todo-forward-rendering: Removed due to maintenance issue.
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
			, numWorkersForAssetStreamer(2)
		{
		}

		int32 windowWidth;
		int32 windowHeight;
		bool fullscreen;
		const char* title;

		ERendererType rendererType;

		uint32 numWorkersForAssetStreamer;

		void(*tick)(float deltaSeconds)     = nullptr;
		void(*render)()                     = nullptr;
	};

	class Engine final {
		friend class EngineUtil;

		using ExecProc = std::function<void(const std::string&)>;
		using GlobalRenderRoutine = std::function<void(class OpenGLDevice* renderDevice)>;

	// Static members
	public:
		static bool init(int argc, char** argv, const EngineConfig& conf);

		// [INTERNAL USE ONLY]
		// Given routine is called right after the render device is initialized.
		// Use for initialization of global resources.
		static void internal_registerGlobalRenderRoutine(GlobalRenderRoutine initRoutine, GlobalRenderRoutine destroyRoutine);

		static std::vector<GlobalRenderRoutine> globalRenderInitRoutines;
		static std::vector<GlobalRenderRoutine> globalRenderDestroyRoutines;

	// Public API
	public:
		void start();
		void stop();

		void registerExec(const char* command, ExecProc proc);
		bool execute(const std::string& command);

		void setWorld(Scene* inScene, Camera* inCamera);

		const EngineConfig& getConfig() const { return conf; }

		inline float getCPUTime() const { return elapsed_gameThread + elapsed_renderThread; } // Currently single-threaded (in milliseconds)
		inline float getGPUTime() const { return elapsed_gpu; } // Estimated time of GPU work (in milliseconds)

		InputSystem* getInputSystem() const { return inputSystem.get(); }

		AssetStreamer* getAssetStreamer() const { return assetStreamer.get(); }

		inline class GUIWindow* getMainWindow() const { return mainWindow.get(); }

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
		bool destroy();

		bool initializeMainWindow(int argcp, char** argv);
		bool initializeInput();
		bool initializeAssetStreamer();
		bool initializeOpenGL();
		bool initializeImageLibrary();
		bool initializeFontSystem();
		bool initializeConsole();
		bool initializeRenderer();

		bool destroyOpenGL();

		// glut event listeners //
		static void onIdle();
		static void onMainWindowDisplay();
		static void onMainWindowReshape(int32 newWidth, int32 newHeight);
		static void onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY);
		static void onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY);
		static void onSpecialKeyDown(InputConstants specialKey);
		static void onSpecialKeyUp(InputConstants specialKey);

	private:
		void tick();
		void render();

	// Game thread
	private:
		EngineConfig conf;
		StackAllocator renderProxyAllocator;
		Stopwatch stopwatch_gameThread;
		Stopwatch stopwatch_renderThread;

		float elapsed_gameThread;
		float elapsed_renderThread;

		Scene* scene;
		Camera* camera;

		std::map<std::string, ExecProc> execMap;

		std::unique_ptr<class InputSystem> inputSystem;

		std::unique_ptr<class GUIWindow> mainWindow;

	// Render thread
	private:
		class OpenGLDevice* render_device;
		Renderer* renderer;

		GLuint timer_query;
		float elapsed_gpu; // in milliseconds

		// System textures
		GLuint texture2D_black = 0;
		GLuint texture2D_white = 0;
		GLuint texture2D_grey  = 0;
		GLuint texture2D_blue  = 0;

	// Utility thread
	private:
		std::unique_ptr<class AssetStreamer> assetStreamer;

	};

	extern Engine* gEngine;
	extern class ConsoleWindow* gConsole;

}
