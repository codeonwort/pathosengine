#pragma once

#include "badger/types/noncopyable.h"
#include "badger/types/int_types.h"

#include "render/render_command_list.h"

namespace pathos {

	class OverlayRenderer;
	class DisplayObject2D;
	class Label;

	// Debug 2D rendering after 3D scene is done (like ConsoleWindow)
	class DebugOverlay : public Noncopyable {

	public:
		DebugOverlay(OverlayRenderer* renderer2D);
		~DebugOverlay();

		void initialize();
		void renderDebugOverlay(RenderCommandList& cmdList, int32 screenWidth, int32 screenHeight);
		
		inline void toggleEnabled() { enabled = !enabled; }
		inline void setEnabled(bool bValue) { enabled = bValue; }
		inline bool getEnabled() const { return enabled; }

		inline void toggleFrameStat() { showFrameStat = !showFrameStat; }

	private:
		bool enabled; // Controls all visibility
		bool showFrameStat;

		OverlayRenderer* renderer;

		DisplayObject2D* root;

		float gameThreadTime;
		float renderThreadTime;
		float gpuTime;
		Label* gameThreadTimeLabel;
		Label* renderThreadTimeLabel;
		Label* gpuTimeLabel;
	};

}
