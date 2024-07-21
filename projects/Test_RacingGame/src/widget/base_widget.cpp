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
	} else {
		inputManager->deactivate(true);
	}
}
