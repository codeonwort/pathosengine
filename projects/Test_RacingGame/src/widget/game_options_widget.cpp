#include "game_options_widget.h"
#include "world/world_racing_title.h"

#include "pathos/overlay/label.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

// #wip: Query available display resolutions and actually change the resolution.
static const int32 RESOLUTION_PRESETS[][2] = {
	{ 1024, 768 },
	{ 1600, 900 },
	{ 1920, 1080 },
};
const int32 NUM_RESOLUTIONS = _countof(RESOLUTION_PRESETS);

GameOptionsWidget::GameOptionsWidget(World_RacingTitle* inOwnerWorld)
	: BaseWidget()
{
	ownerWorld = inOwnerWorld;
	createUI();
	bindInput();
}

void GameOptionsWidget::createUI() {
	const int32 screenWidth = gEngine->getConfig().windowWidth;
	const int32 screenHeight = gEngine->getConfig().windowHeight;

	const float marginX = 60.0f, marginY = 60.0f;
	background = new pathos::Rectangle(screenWidth - marginX, screenHeight - marginY);
	background->setBrush(new SolidColorBrush(0.1f, 0.1f, 0.1f, 0.9f));
	background->setX(0.5f * marginX);
	background->setY(0.5f * marginY);
	addChild(background);

	resolutionHeaderLabel = new pathos::Label(L"Resolution");
	resolutionHeaderLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	resolutionHeaderLabel->setXY(20.0f, 20.0f);
	background->addChild(resolutionHeaderLabel);

	resolutionContentLabel = new pathos::Label(L"1920 x 1080");
	resolutionContentLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	resolutionContentLabel->setXY(resolutionHeaderLabel->getX() + 200.0f, resolutionHeaderLabel->getY());
	background->addChild(resolutionContentLabel);

	backToTitleLabel = new pathos::Label(L"Exit");
	backToTitleLabel->setColor(vector3(1.0f, 0.2f, 0.1f));
	backToTitleLabel->setXY(20.0f, background->getHeight() - 40.0f);
	background->addChild(backToTitleLabel);

	optionItems = { resolutionHeaderLabel, backToTitleLabel };
	selectedItem = 0;
	selectedResolution = NUM_RESOLUTIONS - 1;
	updateUI();
}

void GameOptionsWidget::bindInput() {
	ButtonBinding up({ InputConstants::KEYBOARD_ARROW_UP, InputConstants::XBOXONE_DPAD_UP });
	ButtonBinding down({ InputConstants::KEYBOARD_ARROW_DOWN, InputConstants::XBOXONE_DPAD_DOWN });
	ButtonBinding left({ InputConstants::KEYBOARD_ARROW_LEFT, InputConstants::XBOXONE_DPAD_LEFT });
	ButtonBinding right({ InputConstants::KEYBOARD_ARROW_RIGHT, InputConstants::XBOXONE_DPAD_RIGHT });
	ButtonBinding confirm({ InputConstants::SPACE, InputConstants::XBOXONE_A });

	inputManager->bindButtonPressed("up", up, [this]() {
		selectedItem = (selectedItem - 1 + (int32)optionItems.size()) % optionItems.size();
		updateUI();
	});
	inputManager->bindButtonPressed("down", down, [this]() {
		selectedItem = (selectedItem + 1) % optionItems.size();
		updateUI();
	});
	inputManager->bindButtonPressed("left", left, [this]() {
		if (optionItems[selectedItem] == resolutionHeaderLabel) {
			selectedResolution = std::max(0, selectedResolution - 1);
			updateUI();
		}
	});
	inputManager->bindButtonPressed("right", right, [this]() {
		if (optionItems[selectedItem] == resolutionHeaderLabel) {
			selectedResolution = std::min(selectedResolution + 1, NUM_RESOLUTIONS - 1);
			updateUI();
		}
	});
	inputManager->bindButtonPressed("confirm", confirm, [this]() {
		if (optionItems[selectedItem] == backToTitleLabel) {
			ownerWorld->onCloseOptionsWidget();
		}
	});
}

void GameOptionsWidget::updateUI() {
	{
		wchar_t resolutionMsg[128];
		auto& preset = RESOLUTION_PRESETS[selectedResolution];
		swprintf_s(resolutionMsg, L"<- %d x %d ->", preset[0], preset[1]);
		resolutionContentLabel->setText(resolutionMsg);
	}

	for (auto i = 0; i < optionItems.size(); ++i) {
		if ((int32)i == selectedItem) {
			optionItems[i]->setColor(vector3(1.0f, 1.0f, 1.0f));
			if (optionItems[i] == resolutionHeaderLabel) {
				resolutionContentLabel->setColor(vector3(1.0f, 1.0f, 1.0f));
			}
		} else {
			optionItems[i]->setColor(vector3(0.4f, 0.4f, 0.4f));
			if (optionItems[i] == resolutionHeaderLabel) {
				resolutionContentLabel->setColor(vector3(0.4f, 0.4f, 0.4f));
			}
		}
	}
}
