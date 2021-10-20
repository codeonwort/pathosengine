#include "brush.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/overlay/overlaypass.h"
#include "pathos/util/color_conversion.h"

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////////
	// SolidColorBrush
	SolidColorBrush::SolidColorBrush(float r, float g, float b) {
		color = vector4(r, g, b, 1.0f);
	}

	SolidColorBrush::SolidColorBrush(uint32_t colorHex) {
		float rgb[3];
		to_float3(colorHex, rgb);
		color = vector4(rgb[0], rgb[1], rgb[2], 1.0f);
	}

	class OverlayPass* SolidColorBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Standard* program = renderer->getStandardProgram();
		program->setColor(color);
		return program;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// TextBrush
	TextBrush::TextBrush(float r, float g, float b) {
		color = vector4(r, g, b, 1.0f);
	}

	TextBrush::TextBrush(uint32_t colorHex) {
		float rgb[3];
		to_float3(colorHex, rgb);
		color = vector4(rgb[0], rgb[1], rgb[2], 1.0f);
	}

	class OverlayPass* TextBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Text* program = renderer->getTextProgram();
		program->setColor(color);
		return program;
	}

}
