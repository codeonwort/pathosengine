#include "game_options_widget.h"
#include "world/world_racing_title.h"

#include "pathos/overlay/label.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

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
	background->setBrush(new SolidColorBrush(0.2f, 0.2f, 0.2f, 0.9f));
	background->setX(0.5f * marginX);
	background->setY(0.5f * marginY);
	addChild(background);

	updateUI();
}

void GameOptionsWidget::bindInput() {
	ButtonBinding up, down, confirm;
	up.addInput(InputConstants::KEYBOARD_ARROW_UP);
	down.addInput(InputConstants::KEYBOARD_ARROW_DOWN);
	confirm.addInput(InputConstants::SPACE);

	// #wip
	inputManager->bindButtonPressed("up", up, [this]() {
		//
	});
	inputManager->bindButtonPressed("down", down, [this]() {
		//
	});
	inputManager->bindButtonPressed("confirm", confirm, [this]() {
		//
	});
}

void GameOptionsWidget::updateUI() {
	// #wip
}
