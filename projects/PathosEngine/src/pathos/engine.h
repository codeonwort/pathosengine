#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/gl_debug_group.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/smart_pointer.h"
#include "pathos/input/input_system.h"
#include "pathos/util/sync_event.h"

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/system/mem_alloc.h"
#include "badger/system/stopwatch.h"

#include <map>
#include <list>
#include <vector>
#include <mutex>
#include <memory>
#include <string>
#include <functional>

namespace pathos {

	class OpenGLDevice;
	class RenderThread;
	class GUIWindow;
	class ConsoleWindow;
	class World;
	class InputSystem;
	class AssetStreamer;
	class Renderer;
	class MeshGeometry;
	class Texture;
	class DisplayObject2D;
	class DebugOverlay;
	class OverlaySceneProxy;
	class OverlayRenderer;

	using Screenshot = std::pair<vector2i, uint8*>; // Assumes rgb8 format

	// See Engine::init method.
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

	enum class EngineStatus {
		Uninitialized,
		Initialized,
		Running,
		Destroying,
		Destroyed,
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
		void stop(bool immediate = false);  // Stop and destroy the engine.

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

		// --------------------------------------------------------------------
		// Subsystems

		inline InputSystem*     getInputSystem()   const { return inputSystem.get();    }
		inline AssetStreamer*   getAssetStreamer() const { return assetStreamer.get();  }
		inline GUIWindow*       getMainWindow()    const { return mainWindow.get();     }
		inline DisplayObject2D* getOverlayRoot()   const { return appOverlayRoot.get(); }

		// --------------------------------------------------------------------
		// Common resources

		assetPtr<MeshGeometry> getSystemGeometryUnitPlane() const;
		assetPtr<MeshGeometry> getSystemGeometryUnitCube()  const;

		inline Texture* getSystemTexture2DBlack()      const { return texture2D_black;     }
		inline Texture* getSystemTexture2DWhite()      const { return texture2D_white;     }
		inline Texture* getSystemTexture2DGrey()       const { return texture2D_grey;      }
		inline Texture* getSystemTexture2DBlue()       const { return texture2D_blue;      }
		inline Texture* getSystemTexture2DNormalmap()  const { return texture2D_normalmap; }
		inline Texture* getSystemTextureCubeBlack()    const { return textureCube_black;   }

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
			const GpuCounterResult& inGpuCounterResult);

		void internal_updateBasePassCullStat_renderThread(uint32 totalDrawcall, uint32 culledDrawcall);
		inline uint32 internal_getLastBasePassTotalDrawcall() const { return lastBasePassTotalDrawcall; }
		inline uint32 internal_getLastBasePassCulledDrawcall() const { return lastBasePassCulledDrawcall; }

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
		
		void stopDeferred();

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
		EngineStatus engineStatus = EngineStatus::Uninitialized;
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

		// Stats that are filled by render thread.
		float elapsed_renderThread = 0.0f;
		float elapsed_gpu = 0.0f;
		uint32 lastBasePassTotalDrawcall = 0;
		uint32 lastBasePassCulledDrawcall = 0;
		GpuCounterResult lastGpuCounterResult;
		std::mutex gpuQueryMutex;

		// System geometry
		assetPtr<MeshGeometry> geometry_unitPlane; // width = height = 2
		assetPtr<MeshGeometry> geometry_unitCube;  // half size = (1, 1, 1)

		// System textures
		Texture* texture2D_black     = nullptr;
		Texture* texture2D_white     = nullptr;
		Texture* texture2D_grey      = nullptr;
		Texture* texture2D_blue      = nullptr;
		Texture* texture2D_normalmap = nullptr;
		Texture* textureCube_black   = nullptr;

		std::vector<Screenshot> screenshotQueue;

	// Utility thread
	private:
		uniquePtr<AssetStreamer> assetStreamer;

	};

	extern Engine* gEngine;
	extern ConsoleWindow* gConsole;

}
