#include "brush.h"
#include "pathos/rhi/texture.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/overlay/overlaypass.h"
#include "pathos/util/color_conversion.h"

// SolidColorBrush
namespace pathos {

	SolidColorBrush::SolidColorBrush(float r, float g, float b, float a) {
		setColor(r, g, b, a);
	}

	SolidColorBrush::SolidColorBrush(vector4 rgba) {
		setColor(rgba.r, rgba.g, rgba.b, rgba.a);
	}

	SolidColorBrush::SolidColorBrush(uint32 colorHex) {
		setColor(colorHex);
	}

	OverlayPass* SolidColorBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Standard* program = renderer->getStandardProgram();
		program->setColor(color);
		return program;
	}

	void SolidColorBrush::setColor(float r, float g, float b, float a) {
		color = vector4(r, g, b, a);
	}

	void SolidColorBrush::setColor(uint32 colorHex) {
		float rgba[4];
		pathos::to_float4(colorHex, rgba);
		setColor(rgba[0], rgba[1], rgba[2], rgba[3]);
	}

}

// TextBrush
namespace pathos {
	
	TextBrush::TextBrush(float r, float g, float b) {
		color = vector4(r, g, b, 1.0f);
	}

	TextBrush::TextBrush(uint32 colorHex) {
		float rgb[3];
		pathos::to_float3(colorHex, rgb);
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

	OverlayPass* ImageBrush::configure(OverlayRenderer* renderer, const Transform& transformAccum) {
		OverlayPass_Image* program = renderer->getImageProgram();
		program->setImageTexture(getTexture());
		return program;
	}

	GLuint ImageBrush::getTexture() const {
		return textureName != 0 ? textureName : textureWrapper->internal_getGLName();
	}

}
