#include "player_controller.h"

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/scene/camera.h"
#include "pathos/input/input_manager.h"

#include "badger/math/minmax.h"

void PlayerController::onSpawn() {
	setupInput();
}

void PlayerController::onTick(float deltaSeconds) {
	InputManager* input = getWorld()->getInputManager();

	float moveRight   = deltaSeconds * speedRight;
	float moveForward = deltaSeconds * speedForward;
	float moveUp      = deltaSeconds * speedUp;
	float rotateYaw   = deltaSeconds * speedYaw;
	float rotatePitch = deltaSeconds * speedPitch;

	float deltaRight   = input->getAxis("moveRight") * moveRight;
	float deltaForward = input->getAxis("moveForward") * moveForward;
	float deltaUp      = input->getAxis("moveUp") * moveUp;

	if (target != nullptr) {
		vector3 pos = target->getActorLocation();
		pos.x += deltaRight;
		target->setActorLocation(pos);
	}
}

void PlayerController::setupInput() {
	AxisBinding moveForward;
	moveForward.addInput(InputConstants::KEYBOARD_W, 1.0f);
	moveForward.addInput(InputConstants::KEYBOARD_S, -1.0f);
	moveForward.addInput(InputConstants::XBOXONE_LEFT_THUMB_Y, 1.0f);

	AxisBinding moveRight;
	moveRight.addInput(InputConstants::KEYBOARD_D, 1.0f);
	moveRight.addInput(InputConstants::KEYBOARD_A, -1.0f);
	moveRight.addInput(InputConstants::XBOXONE_LEFT_THUMB_X, 1.0f);

	InputManager* inputManager = getWorld()->getInputManager();
	inputManager->bindAxis("moveForward", moveForward);
	inputManager->bindAxis("moveRight", moveRight);
}
