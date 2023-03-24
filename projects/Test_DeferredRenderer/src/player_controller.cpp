#include "player_controller.h"

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/scene/camera.h"
#include "pathos/input/input_manager.h"

#include "badger/math/minmax.h"

void PlayerController::onSpawn()
{
	setupInput();
}

void PlayerController::onTick(float deltaSeconds)
{
	Camera& camera = getWorld()->getCamera();

	static int32 prevMouseX = 0;
	static int32 prevMouseY = 0;

	InputManager* input = getWorld()->getInputManager();

	int32 currMouseX = input->getMouseX();
	int32 currMouseY = input->getMouseY();

	const float moveMultiplier = speedScale * badger::max(1.0f, input->getAxis("moveFast") * 10.0f);
	float moveRight    = deltaSeconds * (speedRight * moveMultiplier);
	float moveForward  = deltaSeconds * (speedForward * moveMultiplier);
	float moveUp       = deltaSeconds * (speedUp * moveMultiplier);
	float rotateYaw    = deltaSeconds * speedYaw;
	float rotatePitch  = deltaSeconds * speedPitch;

	float deltaRight   = input->getAxis("moveRight")   * moveRight;
	float deltaForward = input->getAxis("moveForward") * moveForward;
	float deltaUp      = input->getAxis("moveUp")      * moveUp;
	float rotY         = 0.1f * (currMouseX - prevMouseX) * rotateYaw;
	float rotX         = 0.1f * (currMouseY - prevMouseY) * rotatePitch;

	camera.moveForward(deltaForward);
	camera.moveRight(deltaRight);
	camera.moveUp(deltaUp);
	if (rotateByMouse) {
		camera.rotateYaw(rotY);
		camera.rotatePitch(rotX);
	}

	rotX = input->getAxis("rotatePitch");
	rotY = input->getAxis("rotateYaw");
	camera.rotateYaw(rotY);
	camera.rotatePitch(rotX);

	prevMouseX = currMouseX;
	prevMouseY = currMouseY;
}

void PlayerController::setupInput()
{
	AxisBinding moveForward;
	moveForward.addInput(InputConstants::KEYBOARD_W, 1.0f);
	moveForward.addInput(InputConstants::KEYBOARD_S, -1.0f);
	moveForward.addInput(InputConstants::XBOXONE_LEFT_THUMB_Y, 1.0f);

	AxisBinding moveRight;
	moveRight.addInput(InputConstants::KEYBOARD_D, 1.0f);
	moveRight.addInput(InputConstants::KEYBOARD_A, -1.0f);
	moveRight.addInput(InputConstants::XBOXONE_LEFT_THUMB_X, 1.0f);

	AxisBinding moveUp;
	moveUp.addInput(InputConstants::KEYBOARD_E, 1.0f);
	moveUp.addInput(InputConstants::KEYBOARD_Q, -1.0f);

	AxisBinding moveFast;
	moveFast.addInput(InputConstants::SHIFT, 1.0f);
	moveFast.addInput(InputConstants::XBOXONE_RIGHT_TRIGGER, 1.0f);

	AxisBinding rotateYaw;
	rotateYaw.addInput(InputConstants::XBOXONE_RIGHT_THUMB_X, 1.0f);

	AxisBinding rotatePitch;
	rotatePitch.addInput(InputConstants::XBOXONE_RIGHT_THUMB_Y, -1.0f);

	ButtonBinding drawShadowFrustum;
	drawShadowFrustum.addInput(InputConstants::KEYBOARD_F);

	ButtonBinding updateSceneCapture;
	updateSceneCapture.addInput(InputConstants::KEYBOARD_G);

	ButtonBinding rmb;
	rmb.addInput(InputConstants::MOUSE_RIGHT_BUTTON);

	InputManager* inputManager = getWorld()->getInputManager();
	inputManager->bindAxis("moveForward", moveForward);
	inputManager->bindAxis("moveRight", moveRight);
	inputManager->bindAxis("moveUp", moveUp);
	inputManager->bindAxis("moveFast", moveFast);
	inputManager->bindAxis("rotateYaw", rotateYaw);
	inputManager->bindAxis("rotatePitch", rotatePitch);
	inputManager->bindButtonPressed("RMB", rmb, [this]() { rotateByMouse = true; });
	inputManager->bindButtonReleased("RMB", rmb, [this]() { rotateByMouse = false; });
}
