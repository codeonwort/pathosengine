#include "base_widget.h"

#include "pathos/engine.h"
#include "pathos/gui/gui_window.h"
#include "pathos/input/input_manager.h"

BaseWidget::BaseWidget() {
	inputManager = gEngine->getInputSystem()->createInputManager();
	inputManager->bindXInput(XInputUserIndex::USER0);

	gEngine->getMainWindow()->getSize(&baseResolutionX, &baseResolutionY);
}

BaseWidget::~BaseWidget() {
	delete inputManager;
}

void BaseWidget::setWidgetEnabled(bool value) {
	setVisible(value);
	if (value) {
		inputManager->activate();
		// When activating a widget, it may handle keyPress events immediately, which is not preferrable.
		// Prevent it by copying key press states from the default input manager.
		inputManager->copyKeyStateFrom(gEngine->getInputSystem()->getDefaultInputManager());
	} else {
		inputManager->deactivate(true);
	}
}

void BaseWidget::fitToScreenSize() {
	uint32 newResolutionX, newResolutionY;
	gEngine->getMainWindow()->getSize(&newResolutionX, &newResolutionY);

	const float scaleX = (float)newResolutionX / (float)baseResolutionX;
	const float scaleY = (float)newResolutionY / (float)baseResolutionY;
	
	const float scale = std::min(scaleX, scaleY);
	setScaleX(scale);
	setScaleY(scale);
}
