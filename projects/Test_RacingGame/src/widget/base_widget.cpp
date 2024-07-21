#include "base_widget.h"

#include "pathos/engine.h"
#include "pathos/input/input_manager.h"

BaseWidget::BaseWidget() {
	inputManager = gEngine->getInputSystem()->createInputManager();
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
