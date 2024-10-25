#pragma once

#include "pathos/overlay/display_object.h"
using namespace pathos;

namespace pathos {
	class InputManager;
}

class BaseWidget : public DisplayObject2D {

public:
	BaseWidget();
	virtual ~BaseWidget();

	// true: visible and receive input.
	// false: invisible and ignore input.
	void setWidgetEnabled(bool value);

	void fitToScreenSize();

protected:
	InputManager* inputManager = nullptr;

	uint32 baseResolutionX, baseResolutionY;

};
