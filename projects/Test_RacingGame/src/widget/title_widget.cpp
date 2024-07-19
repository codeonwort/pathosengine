#include "title_widget.h"
#include "world/world_racing_title.h"

#include "pathos/overlay/label.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

TitleWidget::TitleWidget(World_RacingTitle* inOwnerWorld) {
	ownerWorld = inOwnerWorld;
	createUI();
	bindInput();
}

void TitleWidget::createUI() {
	const int32 screenWidth = gEngine->getConfig().windowWidth;
	const int32 screenHeight = gEngine->getConfig().windowHeight;

	startLabel = new pathos::Label;
	startLabel->setText(L"Start");
	startLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	startLabel->setX(screenWidth / 2.0f);
	startLabel->setY(screenHeight / 2.0f);
	addChild(startLabel);

	settingsLabel = new pathos::Label;
	settingsLabel->setText(L"Settings");
	settingsLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	settingsLabel->setX(screenWidth / 2.0f);
	settingsLabel->setY(startLabel->getY() + 20.0f);
	addChild(settingsLabel);

	exitLabel = new pathos::Label;
	exitLabel->setText(L"Exit");
	exitLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	exitLabel->setX(screenWidth / 2.0f);
	exitLabel->setY(settingsLabel->getY() + 20.0f);
	addChild(exitLabel);

	labels = { startLabel, settingsLabel, exitLabel };
	selectedLabel = 0;
	updateUI();
}

void TitleWidget::bindInput() {
	ButtonBinding up, down, confirm;
	up.addInput(InputConstants::KEYBOARD_ARROW_UP);
	down.addInput(InputConstants::KEYBOARD_ARROW_DOWN);
	confirm.addInput(InputConstants::SPACE);

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindButtonPressed("up", up, [this]() {
		selectedLabel = (selectedLabel - 1 + (int32)labels.size()) % labels.size();
		updateUI();
	});
	inputManager->bindButtonPressed("down", down, [this]() {
		selectedLabel = (selectedLabel + 1) % labels.size();
		updateUI();
	});
	inputManager->bindButtonPressed("confirm", confirm, [this]() {
		if (labels[selectedLabel] == startLabel) {
			LOG(LogDebug, "Start Game");
			ownerWorld->onStartGame();
		} else if (labels[selectedLabel] == settingsLabel) {
			// #wip: Implement settings widget.
			LOG(LogDebug, "WIP: Settings");
		} else if (labels[selectedLabel] == exitLabel) {
			// #wip: Notify the game engine to terminate.
			LOG(LogDebug, "WIP: Exit");
		} else {
			LOG(LogError, "Unknown label index: %d", selectedLabel);
		}
	});
}

void TitleWidget::updateUI() {
	for (auto i = 0; i < labels.size(); ++i) {
		if ((int32)i == selectedLabel) {
			labels[i]->setColor(vector3(1.0f, 1.0f, 1.0f));
		} else {
			labels[i]->setColor(vector3(0.2f, 0.2f, 0.2f));
		}
	}
}
