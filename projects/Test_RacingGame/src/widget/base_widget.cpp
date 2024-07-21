#include "base_widget.h"

#include "pathos/engine.h"
#include "pathos/input/input_manager.h"

BaseWidget::BaseWidget() {
	inputManager = gEngine->getInputSystem()->createInputManager();
	inputManager->bindXInput(XInputUserIndex::USER0);
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
