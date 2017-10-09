#pragma once

#include "pathos/overlay/display_object.h"
#include "pathos/wrapper/transform.h"
#include "pathos/render/overlay/overlaypass.h"

namespace pathos {

	// Renders the overlay (2D display over 3D scene)
	class OverlayRenderer {

	public:
		OverlayRenderer();
		OverlayRenderer(const OverlayRenderer& other) = delete;
		OverlayRenderer(OverlayRenderer&&) = delete;
		virtual ~OverlayRenderer();

		void render(DisplayObject2D* root);
		void render_recurse(DisplayObject2D* object, const Transform& transformAccum);

	protected:
		void createShaders();
		void destroyShaders();

		// render passes
		OverlayPass_Standard* renderpass_standard = nullptr;

		// temporary variables (boring to pass them as arguments...)
		DisplayObject2D* root = nullptr;

	private:
		Transform toNDC; // screen coordinates to NDC
		void calculateTransformNDC();

	};

}