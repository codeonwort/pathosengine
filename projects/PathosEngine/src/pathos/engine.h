#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/smart_pointer.h"
#include "pathos/input/input_system.h"
#include "pathos/util/sync_event.h"

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
	class OverlaySceneProxy;
	class ConsoleWindow;
	class AssetStreamer;
	class OverlayRenderer;
	class RenderThread;
	class DisplayObject2D;
	class MeshGeometry;

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

	// Engine public API
	public:
		// Initialize an engine instance. Program entry point should call this prior to everything.
		static bool init(int argc, char** argv, const EngineConfig& conf);

		void start(); // Start the engine main loop.
		void stop();  // Stop and destroy the engine.

		// proc = [](const std::string& command) { ... }
		void registerConsoleCommand(const char* command, ExecProc proc);
		// @return true if it's a registered command. Wether the command was executed successfully is not considered.
		bool executeConsoleCommand(const std::string& command);

		// Toggle debug overlay for frame stats.
		void toggleFrameStat();

		// Dump GPU profile to a text file.
		void dumpGPUProfile();

		// Change current world.
		void setWorld(World* inWorld);

		inline const EngineConfig& getConfig() const { return conf; }

		inline float getWorldTime()           const { return 0.001f * stopwatch_app.stop(); } // Elapsed seconds since the application started.
		inline float getGameThreadCPUTime()   const { return elapsed_gameThread;            }
		inline float getRenderThreadCPUTime() const { return elapsed_renderThread;          }
		inline float getGPUTime()             const { return elapsed_gpu;                   } // Estimated time of GPU work (in milliseconds)

		inline InputSystem*     getInputSystem()   const { return inputSystem.get();    }
		inline AssetStreamer*   getAssetStreamer() const { return assetStreamer.get();  }
		inline GUIWindow*       getMainWindow()    const { return mainWindow.get();     }
		inline DisplayObject2D* getOverlayRoot()   const { return appOverlayRoot.get(); }

		inline MeshGeometry* getSystemGeometryUnitPlane() const { return geometry_unitPlane; }
		inline MeshGeometry* getSystemGeometryUnitCube() const { return geometry_unitCube; }

		inline GLuint getSystemTexture2DBlack()      const { return texture2D_black;     }
		inline GLuint getSystemTexture2DWhite()      const { return texture2D_white;     }
		inline GLuint getSystemTexture2DGrey()       const { return texture2D_grey;      }
		inline GLuint getSystemTexture2DBlue()       const { return texture2D_blue;      }
		inline GLuint getSystemTexture2DNormalmap()  const { return texture2D_normalmap; }
		inline GLuint getSystemTextureCubeBlack()    const { return textureCube_black;   }

	// Public but engine internal use only
	public:
		// Given routine is called right after the render device is initialized.
		// Use for initialization of global resources.
		class GlobalRenderRoutineContainer {
		public:
			std::mutex vector_mutex;
			std::vector<GlobalRenderRoutine> initRoutines;
			std::vector<GlobalRenderRoutine> destroyRoutines;
		};
		static GlobalRenderRoutineContainer& internal_getGlobalRenderRoutineContainer();
		static void internal_registerGlobalRenderRoutine(GlobalRenderRoutine initRoutine, GlobalRenderRoutine destroyRoutine);

		void internal_updateScreenSize(int32 inScreenWidth, int32 inScreenHeight);

		void internal_pushSceneProxy(SceneProxy* newSceneProxy);
		void internal_pushOverlayProxy(OverlaySceneProxy* newOverlayProxy);

		void internal_updateMainWindow_renderThread();

		void internal_updateGPUQuery_renderThread(
			float inElapsedRenderThread,
			float inElapsedGpu,
			const std::vector<std::string>& inGpuCounterNames,
			const std::vector<float>& inGpuCounterTimes);

		void internal_getLastGPUCounters(
			std::vector<std::string>& outGpuCounterNames,
			std::vector<float>& outGpuCounterTimes);

		// Called by render thread when a screenshot is taken.
		void internal_pushScreenshot(Screenshot screenshot);

		inline const std::map<std::string, ExecProc>& internal_getExecMap() const { return execMap; }

	private:
		Engine() = default;
		~Engine() = default;

		bool initialize(int argcp, char** argv, const EngineConfig& conf);

		bool initializeMainWindow(int argcp, char** argv);
		bool initializeInput();
		bool initializeAssetStreamer();
		
		bool initializeImageLibrary();
		bool initializeFontSystem(RenderCommandList& cmdList);
		bool initializeConsole();

		void readConfigFile(const char* configFilename, std::vector<std::string>& outEffectiveLines);

		void tickMainThread();

		// GUI event listeners
		// - GLUT callbacks -> GUIWindow callbacks -> engine callbacks (here).
		// - These callbacks are grouped by GUIWindowCreateParams and passed to GUIWindow.
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
		static void onMouseDrag(int32 mouseX, int32 mouseY);

	// Game thread
	private:
		EngineConfig conf;
		Stopwatch stopwatch_gameThread;
		Stopwatch stopwatch_app;

		std::vector<SceneProxy*> reservedSceneProxies;
		std::vector<OverlaySceneProxy*> reservedOverlayProxies;
		uniquePtr<Fence> frameFence;
		uint32 frameNumber_mainThread = 0; // Set to 1 in initialize()
		float elapsed_gameThread = 0.0f;

		World* currentWorld = nullptr;
		World* pendingNewWorld = nullptr;

		std::map<std::string, ExecProc> execMap;

		uniquePtr<InputSystem> inputSystem;

		uniquePtr<GUIWindow> mainWindow;

		// Root for 2D objects that are created by application.
		// ConsoleWindow and DebugOverlay have their own roots.
		uniquePtr<DisplayObject2D> appOverlayRoot;

	// Render thread
	private:
		RenderThread* renderThread = nullptr;

		// Render thread will fill in these
		float elapsed_renderThread = 0.0f;
		float elapsed_gpu = 0.0f;
		std::vector<std::string> lastGpuCounterNames;
		std::vector<float> lastGpuCounterTimes;
		std::mutex gpuQueryMutex;

		// System geometry
		MeshGeometry* geometry_unitPlane = nullptr; // width = height = 2
		MeshGeometry* geometry_unitCube = nullptr;  // half size = (1, 1, 1)

		// System textures
		GLuint texture2D_black     = 0;
		GLuint texture2D_white     = 0;
		GLuint texture2D_grey      = 0;
		GLuint texture2D_blue      = 0;
		GLuint texture2D_normalmap = 0;
		GLuint textureCube_black   = 0;

		std::vector<Screenshot> screenshotQueue;

	// Utility thread
	private:
		uniquePtr<AssetStreamer> assetStreamer;

	};

	extern Engine* gEngine;
	extern ConsoleWindow* gConsole;

}
