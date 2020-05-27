#pragma once

#include "pathos/wrapper/transform.h"

namespace pathos {

	class OverlayRenderer;
	class OverlayPass;

	// Base class
	class Brush {
	public:
		virtual class OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) = 0;
	};

	// Fill the object with a solid color
	class SolidColorBrush : public Brush {
	public:
		SolidColorBrush(float r, float g, float b);
		SolidColorBrush(uint32_t rgb);
		virtual OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) override;
	private:
		float rgb[4];
	};

	/*
	// TODO: use texture as brush
	class ImageBrush : public Brush {
	public:
		virtual class OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) override;
	};
	*/

	// Brush for Label
	class TextBrush : public Brush {
	public:
		TextBrush(float r, float g, float b);
		TextBrush(uint32_t rgb);
		virtual OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) override;
	private:
		float rgb[4];
	};

}
