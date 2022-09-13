#pragma once

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"

#include "pathos/render/render_command_list.h"
#include "pathos/smart_pointer.h"

#include <vector>

namespace pathos {

	class OverlayRenderer;
	class DisplayObject2D;
	class DisplayObject2DProxy;
	class Label;
	class Rectangle;

	// Debug 2D rendering after 3D scene is done (like ConsoleWindow)
	class DebugOverlay : public Noncopyable {

	public:
		DebugOverlay(OverlayRenderer* renderer2D);
		~DebugOverlay();

		void initialize();

		void renderDebugOverlay(
			RenderCommandList& cmdList,
			DisplayObject2DProxy* rootProxy,
			int32 screenWidth,
			int32 screenHeight);
		
		inline void toggleEnabled() { enabled = !enabled; }
		inline void setEnabled(bool bValue) { enabled = bValue; }
		inline bool getEnabled() const { return enabled; }

		inline void toggleFrameStat() { showFrameStat = !showFrameStat; }

		inline DisplayObject2D* internal_getRoot() const { return root.get(); }

	private:
		OverlayRenderer* renderer;

		bool enabled = true; // Controls all visibility
		bool showFrameStat = false;

		float gameThreadTime = 0.0f;
		float renderThreadTime = 0.0f;
		float gpuTime = 0.0f;

		uniquePtr<DisplayObject2D> root;
		Label* gameThreadTimeLabel = nullptr;
		Label* renderThreadTimeLabel = nullptr;
		Label* gpuTimeLabel = nullptr;
		Rectangle* cycleCounterBackground = nullptr;
		std::vector<Label*> cycleCounterNames;
		std::vector<Label*> cycleCounterValues;
	};

}
