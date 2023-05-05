#include "button.h"
#include "rectangle.h"
#include "label.h"
#include "brush.h"

namespace pathos {

	Button::Button(float width, float height, float paddingX, float paddingY) {
		background = new Rectangle(width, height);
		background->setBrush(backgroundBrush = new SolidColorBrush(1.0f, 1.0f, 1.0f));

		label = new Label();
		label->setX(paddingX);
		label->setY(paddingX);

		addChild(background);
		addChild(label);

		bReceivesMouseInput = true;
		bStopInputPropagation = true;
	}

	Button::~Button() {
		delete backgroundBrush;
	}

	bool Button::onMouseHitTest(int32 mouseX, int32 mouseY) const {
		return background->onMouseHitTest(mouseX - (int32)getX(), mouseY - (int32)getY());
	}

	void Button::setBackgroundColor(float r, float g, float b) {
		backgroundBrush->setColor(r, g, b);
	}

	void Button::setText(const wchar_t* txt) {
		label->setText(txt);
	}

	void Button::setTextColor(float r, float g, float b) {
		label->setColor(vector3(r, g, b));
	}

}
