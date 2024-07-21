#include "title_widget.h"
#include "world/world_racing_title.h"

#include "pathos/overlay/label.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

TitleWidget::TitleWidget(World_RacingTitle* inOwnerWorld)
	: BaseWidget()
{
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

	optionsLabel = new pathos::Label;
	optionsLabel->setText(L"Options");
	optionsLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	optionsLabel->setX(screenWidth / 2.0f);
	optionsLabel->setY(startLabel->getY() + 20.0f);
	addChild(optionsLabel);

	exitLabel = new pathos::Label;
	exitLabel->setText(L"Exit");
	exitLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	exitLabel->setX(screenWidth / 2.0f);
	exitLabel->setY(optionsLabel->getY() + 20.0f);
	addChild(exitLabel);

	labels = { startLabel, optionsLabel, exitLabel };
	selectedLabel = 0;
	updateUI();
}

void TitleWidget::bindInput() {
	ButtonBinding up, down, confirm;
	up.addInput(InputConstants::KEYBOARD_ARROW_UP);
	down.addInput(InputConstants::KEYBOARD_ARROW_DOWN);
	confirm.addInput(InputConstants::SPACE);

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
			LOG(LogDebug, "[Game] Start game world");
			ownerWorld->onStartGameWorld();
		} else if (labels[selectedLabel] == optionsLabel) {
			LOG(LogDebug, "[Game] Open options widget");
			ownerWorld->onOpenOptionsWidget();
		} else if (labels[selectedLabel] == exitLabel) {
			LOG(LogDebug, "[Game] Exit the program");
			gEngine->stop();
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
