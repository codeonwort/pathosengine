#pragma once

#include "display_object.h"

namespace pathos {

	class Rectangle;
	class Label;
	class SolidColorBrush;

	class Button : public DisplayObject2D {

	public:
		Button(float width, float height, float paddingX, float paddingY);
		virtual ~Button();

		// User input
		virtual bool onMouseHitTest(int32 mouseX, int32 mouseY) const override;

		void setBackgroundColor(float r, float g, float b);

		void setText(const wchar_t* txt);
		void setTextColor(float r, float g, float b);

	private:
		Rectangle* background = nullptr;
		SolidColorBrush* backgroundBrush = nullptr;
		Label* label = nullptr;

	};

}
