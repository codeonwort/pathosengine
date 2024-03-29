#pragma once

#include "pathos/rhi/render_command_list.h"
#include "pathos/util/transform_helper.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class OverlayPass_Standard;
	class OverlayPass_Text;
	class OverlayPass_Image;
	class DisplayObject2D;
	class DisplayObject2DProxy;

	// Renders the overlay (2D display over 3D scene)
	class OverlayRenderer : public Noncopyable {

	public:
		OverlayRenderer();
		virtual ~OverlayRenderer();

		void renderOverlay(RenderCommandList& cmdList, DisplayObject2DProxy* inRootProxy);
		void render_recurse(RenderCommandList& cmdList, DisplayObject2DProxy* objectProxy, const Transform& transformAccum);

		// Brushes retrieve desired shader programs in the renderer, configure them, and draw the geometry.
		inline OverlayPass_Standard* getStandardProgram() const { return renderpass_standard; }
		inline OverlayPass_Text* getTextProgram() const { return renderpass_text; }
		inline OverlayPass_Image* getImageProgram() const { return renderpass_image; }

	protected:
		void createShaders();
		void destroyShaders();

		// Render passes
		OverlayPass_Standard* renderpass_standard = nullptr;
		OverlayPass_Text* renderpass_text = nullptr;
		OverlayPass_Image* renderpass_image = nullptr;

		// Temporary variables (nullified after rendering is done each frame)
		DisplayObject2DProxy* root = nullptr;

	private:
		void calculateTransformNDC(uint32 windowWidth, uint32 windowHeight);

		Transform toNDC; // screen coordinates to NDC

	};

}
