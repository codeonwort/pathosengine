#pragma once

#include "pathos/util/transform_helper.h"
#include "badger/types/vector_types.h"
#include "badger/types/int_types.h"

namespace pathos {

	class OverlayRenderer;
	class OverlayPass;

	// Base class
	class Brush {
	public:
		virtual OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) = 0;
	};

	// Fill the object with a solid color
	class SolidColorBrush : public Brush {
	public:
		SolidColorBrush(float r, float g, float b, float a = 1.0f);
		SolidColorBrush(vector4 rgba);
		SolidColorBrush(uint32 rgba);
		virtual OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) override;
		void setColor(float r, float g, float b, float a = 1.0f);
		void setColor(uint32 colorHex);
	private:
		vector4 color;
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
		TextBrush(uint32 rgb);

		virtual OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) override;

		inline void setColor(const vector3& inColor) { color = vector4(inColor, 1.0f); }

	private:
		vector4 color;

	};

	class ImageBrush : public Brush {

	public:
		ImageBrush(GLuint texture);

		virtual OverlayPass* configure(OverlayRenderer* renderer, const Transform& transformAccum) override;

		inline GLuint getTexture() const { return texture; }
		inline void setTexture(GLuint inTexture) { texture = inTexture; }

	private:
		GLuint texture;

	};

}
