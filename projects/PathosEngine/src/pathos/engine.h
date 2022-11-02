#pragma once

#include "pathos/gl_handles.h"
#include "pathos/smart_pointer.h"
#include "pathos/input/input_system.h"
#include "pathos/render/render_command_list.h"

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/memory/mem_alloc.h"
#include "badger/system/stopwatch.h"

#include <map>
#include <list>
#include <vector>
#include <mutex>
#include <memory>
#include <string>
#include <functional>

namespace pathos {

	class World;
	class Renderer;
	class GUIWindow;
	class InputSystem;
	class OpenGLDevice;
	class DebugOverlay;
	class ConsoleWindow;
	class AssetStreamer;
	class OverlayRenderer;
	class RenderThread;

	using Screenshot = std::pair<vector2i, uint8*>;

	// @see Engine::init
	struct EngineConfig {
		EngineConfig()
			: windowWidth(1920)
			, windowHeight(1080)
			, fullscreen(false)
			, title("pathos engine")
			, numWorkersForAssetStreamer(2)
		{
		}

		int32 windowWidth;
		int32 windowHeight;
		bool fullscreen;
		const char* title;

		uint32 numWorkersForAssetStreamer;
	};

	class Engine final : public Noncopyable {
		friend class EngineUtil;
		friend class RenderThread;

		using ExecProc = std::function<void(const std::string&)>; // Parameter is the console input as is
		using GlobalRenderRoutine = std::function<void(OpenGLDevice* renderDevice, RenderCommandList& cmdList)>;

	// Static members
	public:
		static bool init(int argc, char** argv, const EngineConfig& conf);

		// [INTERNAL USE ONLY]
		// Given routine is called right after the render device is initialized.
		// Use for initialization of global resources.
		class GlobalRenderRoutineContainer {
		public:
			std::mutex vector_mutex;
			std::vector<GlobalRenderRoutine> initRoutines;
			std::vector<GlobalRenderRoutine> destroyRoutines;
		};
		static GlobalRenderRoutineContainer& getGlobalRenderRoutineContainer();
		static void internal_registerGlobalRenderRoutine(GlobalRenderRoutine initRoutine, GlobalRenderRoutine destroyRoutine);

	// Public API
	public:
		void start();
		void stop();

		// #todo-renderthread: Ad-hoc communication to render thread. Needs clearer interface.
		void pushSceneProxy(SceneProxy* newSceneProxy);
		void updateMainWindow_renderThread();
		void updateGPUQuery_renderThread(
			float inElapsedRenderThread,
			float inElapsedGpu,
			const std::vector<std::string>& inGpuCounterNames,
			const std::vector<float>& inGpuCounterTimes);

		void registerExec(const char* command, ExecProc proc);
		bool execute(const std::string& command);

		void toggleFrameStat();
		void dumpGPUProfile();
		void getLastGPUCounters(
			std::vector<std::string>& outGpuCounterNames,
			std::vector<float>& outGpuCounterTimes);

		void setWorld(World* inWorld);

		const EngineConfig& getConfig() const { return conf; }
		void updateScreenSize(int32 inScreenWidth, int32 inScreenHeight);

		// Called by render thread when a screenshot is taken.
		void pushScreenshot(Screenshot screenshot);

		inline float getWorldTime() { return stopwatch_app.stop(); } // Elapsed seconds since the application started.
		inline float getGameThreadCPUTime() const { return elapsed_gameThread; }
		inline float getRenderThreadCPUTime() const { return elapsed_renderThread; }
		inline float getGPUTime() const { return elapsed_gpu; } // Estimated time of GPU work (in milliseconds)

		InputSystem* getInputSystem() const { return inputSystem.get(); }

		AssetStreamer* getAssetStreamer() const { return assetStreamer.get(); }

		inline GUIWindow* getMainWindow() const { return mainWindow.get(); }

		inline GLuint getSystemTexture2DBlack()      const { return texture2D_black;     }
		inline GLuint getSystemTexture2DWhite()      const { return texture2D_white;     }
		inline GLuint getSystemTexture2DGrey()       const { return texture2D_grey;      }
		inline GLuint getSystemTexture2DBlue()       const { return texture2D_blue;      }
		inline GLuint getSystemTexture2DNormalmap()  const { return texture2D_normalmap; }

		inline const std::map<std::string, ExecProc>& getExecMap() const { return execMap; }

	private:
		Engine() = default;
		~Engine() = default;

		bool initialize(int argcp, char** argv, const EngineConfig& conf);
		bool destroy();

		bool initializeMainWindow(int argcp, char** argv);
		bool initializeInput();
		bool initializeAssetStreamer();
		
		bool initializeImageLibrary();
		bool initializeFontSystem(RenderCommandList& cmdList);
		bool initializeConsole();

		void readConfigFile(const char* configFilename, std::vector<std::string>& outEffectiveLines);

		// GUI event listeners //
		static void onCloseWindow();
		static void onIdle();
		static void onMainWindowDisplay();
		static void onMainWindowReshape(int32 newWidth, int32 newHeight);
		static void onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY);
		static void onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY);
		static void onSpecialKeyDown(InputConstants specialKey);
		static void onSpecialKeyUp(InputConstants specialKey);
		static void onMouseDown(InputConstants mouseInput, int32 mouseX, int32 mouseY);
		static void onMouseUp(InputConstants mouseInput, int32 mouseX, int32 mouseY);

	private:
		void tick();

	// Game thread
	private:
		EngineConfig conf;
		Stopwatch stopwatch_gameThread;
		Stopwatch stopwatch_app;

		uint32 frameCounter_gameThread = 0;
		float elapsed_gameThread = 0.0f;

		World* currentWorld = nullptr;
		World* pendingNewWorld = nullptr;

		std::map<std::string, ExecProc> execMap;

		uniquePtr<InputSystem> inputSystem;

		uniquePtr<GUIWindow> mainWindow;

	// Render thread
	private:
		RenderThread* renderThread = nullptr;

		// Render thread will fill in these
		float elapsed_renderThread = 0.0f;
		float elapsed_gpu = 0.0f;
		std::vector<std::string> lastGpuCounterNames;
		std::vector<float> lastGpuCounterTimes;
		std::mutex gpuQueryMutex;

		// System textures
		GLuint texture2D_black     = 0;
		GLuint texture2D_white     = 0;
		GLuint texture2D_grey      = 0;
		GLuint texture2D_blue      = 0;
		GLuint texture2D_normalmap = 0;

		std::vector<Screenshot> screenshotQueue;

	// Utility thread
	private:
		uniquePtr<AssetStreamer> assetStreamer;

	};

	extern Engine* gEngine;
	extern ConsoleWindow* gConsole;

}
