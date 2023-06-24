#include "brush.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/overlay/overlaypass.h"
#include "pathos/util/color_conversion.h"

// SolidColorBrush
namespace pathos {

	SolidColorBrush::SolidColorBrush(float r, float g, float b) {
		setColor(r, g, b);
	}

	SolidColorBrush::SolidColorBrush(uint32 colorHex) {
		setColor(colorHex);
	}

	OverlayPass* SolidColorBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Standard* program = renderer->getStandardProgram();
		program->setColor(color);
		return program;
	}

	void SolidColorBrush::setColor(float r, float g, float b) {
		color = vector4(r, g, b, 1.0f);
	}

	void SolidColorBrush::setColor(uint32 colorHex) {
		float rgb[3];
		to_float3(colorHex, rgb);
		setColor(rgb[0], rgb[1], rgb[2]);
	}

}

// TextBrush
namespace pathos {
	
	TextBrush::TextBrush(float r, float g, float b) {
		color = vector4(r, g, b, 1.0f);
	}

	TextBrush::TextBrush(uint32 colorHex) {
		float rgb[3];
		to_float3(colorHex, rgb);
		color = vector4(rgb[0], rgb[1], rgb[2], 1.0f);
	}

	OverlayPass* TextBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Text* program = renderer->getTextProgram();
		program->setColor(color);
		return program;
	}

}

// ImageBrush
namespace pathos {

	ImageBrush::ImageBrush(GLuint inTexture) {
		texture = inTexture;
	}

	OverlayPass* ImageBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Image* program = renderer->getImageProgram();
		program->setImageTexture(texture);
		return program;
	}

}
