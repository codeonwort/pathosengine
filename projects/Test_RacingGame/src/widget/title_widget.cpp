#include "title_widget.h"
#include "world/world_racing_title.h"

#include "pathos/overlay/label.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

// Constants
static const std::string LABEL_FONT = "defaultLarge";
static const float LABEL_SPACE_Y = 50.0f;
static const vector3 LABEL_COLOR(0.4f, 0.4f, 0.4f);
static const vector3 LABEL_COLOR_FOCUS(1.0f, 1.0, 1.0);

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
	startLabel->setFont(LABEL_FONT);
	startLabel->setText(L"Start");
	startLabel->setX((screenWidth - (int32)startLabel->getTextWidth()) / 2.0f);
	startLabel->setY(screenHeight / 2.0f);
	addChild(startLabel);

	optionsLabel = new pathos::Label;
	optionsLabel->setFont(LABEL_FONT);
	optionsLabel->setText(L"Options");
	optionsLabel->setX((screenWidth - (int32)optionsLabel->getTextWidth()) / 2.0f);
	optionsLabel->setY(startLabel->getY() + LABEL_SPACE_Y);
	addChild(optionsLabel);

	exitLabel = new pathos::Label;
	exitLabel->setFont(LABEL_FONT);
	exitLabel->setText(L"Exit");
	exitLabel->setX((screenWidth - (int32)exitLabel->getTextWidth()) / 2.0f);
	exitLabel->setY(optionsLabel->getY() + LABEL_SPACE_Y);
	addChild(exitLabel);

	labels = { startLabel, optionsLabel, exitLabel };
	selectedLabel = 0;
	updateUI();
}

void TitleWidget::bindInput() {
	ButtonBinding up({ InputConstants::KEYBOARD_ARROW_UP, InputConstants::XBOXONE_DPAD_UP });
	ButtonBinding down({ InputConstants::KEYBOARD_ARROW_DOWN, InputConstants::XBOXONE_DPAD_DOWN });
	ButtonBinding confirm({ InputConstants::SPACE, InputConstants::XBOXONE_A });

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
			labels[i]->setColor(LABEL_COLOR_FOCUS);
		} else {
			labels[i]->setColor(LABEL_COLOR);
		}
	}
}
