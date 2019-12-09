#include "brush.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/overlay/overlaypass.h"
#include "pathos/util/color_conversion.h"

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////////
	// SolidColorBrush
	SolidColorBrush::SolidColorBrush(float r, float g, float b) {
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
		rgb[3] = 1.0f;
	}

	SolidColorBrush::SolidColorBrush(uint32_t color) {
		to_float3(color, rgb);
		rgb[3] = 1.0f;
	}

	class OverlayPass* SolidColorBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Standard* program = renderer->getStandardProgram();
		program->setUniform_color(rgb);
		return program;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// TextBrush
	TextBrush::TextBrush(float r, float g, float b) {
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
		rgb[3] = 1.0f;
	}

	TextBrush::TextBrush(uint32_t color) {
		to_float3(color, rgb);
		rgb[3] = 1.0f;
	}

	class OverlayPass* TextBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Text* program = renderer->getTextProgram();
		program->setUniform_color(rgb);
		return program;
	}

}
