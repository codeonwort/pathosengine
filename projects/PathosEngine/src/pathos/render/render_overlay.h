#pragma once

#include "render_command_list.h"
#include "pathos/wrapper/transform.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class OverlayPass_Standard;
	class OverlayPass_Text;
	class DisplayObject2D;

	// Renders the overlay (2D display over 3D scene)
	class OverlayRenderer : public Noncopyable {

	public:
		OverlayRenderer();
		virtual ~OverlayRenderer();

		void renderOverlay(RenderCommandList& cmdList, class DisplayObject2D* inRoot);
		void render_recurse(RenderCommandList& cmdList, class DisplayObject2D* object, const Transform& transformAccum);

		// Brushes retrieve desired shader programs in the renderer, configure them, and draw the geometry.
		inline OverlayPass_Standard* getStandardProgram() const { return renderpass_standard; }
		inline OverlayPass_Text* getTextProgram() const { return renderpass_text; }

	protected:
		void createShaders();
		void destroyShaders();

		// Render passes
		OverlayPass_Standard* renderpass_standard = nullptr;
		OverlayPass_Text* renderpass_text = nullptr;

		// Temporary variables (nullified after rendering is done each frame)
		DisplayObject2D* root = nullptr;

	private:
		void calculateTransformNDC(uint16 windowWidth, uint16 windowHeight);

		Transform toNDC; // screen coordinates to NDC

	};

}
