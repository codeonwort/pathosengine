#pragma once

#include "render_command_list.h"
#include "pathos/wrapper/transform.h"

namespace pathos {

	// Renders the overlay (2D display over 3D scene)
	class OverlayRenderer {

	public:
		OverlayRenderer();
		OverlayRenderer(const OverlayRenderer& other) = delete;
		OverlayRenderer(OverlayRenderer&&) = delete;
		virtual ~OverlayRenderer();

		void renderOverlay(RenderCommandList& cmdList, class DisplayObject2D* inRoot);
		void render_recurse(RenderCommandList& cmdList, class DisplayObject2D* object, const Transform& transformAccum);

		// Brushes retrieve desired programs in the renderer, configure them, and draw the geometry.
		inline class OverlayPass_Standard* getStandardProgram() const { return renderpass_standard; }
		inline class OverlayPass_Text* getTextProgram() const { return renderpass_text; }

	protected:
		void createShaders();
		void destroyShaders();

		// render passes
		class OverlayPass_Standard* renderpass_standard = nullptr;
		class OverlayPass_Text* renderpass_text = nullptr;

		// temporary variables (boring to pass them as arguments...)
		class DisplayObject2D* root = nullptr;

	private:
		Transform toNDC; // screen coordinates to NDC
		void calculateTransformNDC(uint16 windowWidth, uint16 windowHeight);

	};

}
