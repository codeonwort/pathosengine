#pragma once

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"

#include "pathos/rhi/render_command_list.h"
#include "pathos/smart_pointer.h"

namespace pathos {

	class OverlayRenderer;
	class DisplayObject2D;
	class DisplayObject2DProxy;
	class Label;
	class Rectangle;

	class CullStatList;
	class CounterList;

	// Debug 2D rendering after 3D scene is done (like ConsoleWindow)
	class DebugOverlay : public Noncopyable {

	public:
		DebugOverlay(OverlayRenderer* renderer2D);
		~DebugOverlay();

		void initialize();

		void renderDebugOverlay(RenderCommandList& cmdList, DisplayObject2DProxy* rootProxy, int32 screenWidth, int32 screenHeight);
		
		inline void toggleEnabled() { bEnabled = !bEnabled; }
		inline void setEnabled(bool bValue) { bEnabled = bValue; }
		inline bool getEnabled() const { return bEnabled; }

		inline void toggleFrameStat() { bShowFrameStat = !bShowFrameStat; }

		inline DisplayObject2D* internal_getRoot() const { return root.get(); }

	private:
		OverlayRenderer* renderer;

		bool bEnabled = true; // Controls all visibility
		bool bShowFrameStat = false;
		float lastStatUpdateTime = 0.0f;

		float gameThreadTime = 0.0f;
		float renderThreadTime = 0.0f;
		float gpuTime = 0.0f;

		uniquePtr<DisplayObject2D> root;
		Label* gameThreadTimeLabel = nullptr;
		Label* renderThreadTimeLabel = nullptr;
		Label* gpuTimeLabel = nullptr;
		Label* resolutionLabel = nullptr;
		CullStatList* basePassCullList = nullptr;
		CounterList* cpuCounterList = nullptr;
		CounterList* gpuCounterList = nullptr;
	};

}
