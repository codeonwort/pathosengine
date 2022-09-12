#include "player_controller.h"

#include "pathos/engine.h"
#include "pathos/actor/world.h"
#include "pathos/camera/camera.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/log.h"

void PlayerController::onSpawn()
{
	setupInput();
}

void PlayerController::onTick(float deltaSeconds)
{
	if (inPhotoMode) {
		tickPhotoMode(deltaSeconds);
	} else {
		tickGameplay(deltaSeconds);
	}
}

void PlayerController::setPlayerPawn(Actor* player)
{
	playerPawn = player;
}

void PlayerController::togglePhotoMode()
{
	inPhotoMode = !inPhotoMode;
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
	inputManager->bindUniqueAxis("moveForward", moveForward);
	inputManager->bindUniqueAxis("moveRight", moveRight);
	inputManager->bindUniqueAxis("moveUp", moveUp);
	inputManager->bindUniqueAxis("moveFast", moveFast);
	inputManager->bindUniqueAxis("rotateYaw", rotateYaw);
	inputManager->bindUniqueAxis("rotatePitch", rotatePitch);
	inputManager->bindUniqueButtonPressed("RMB", rmb, [this]() { rotateByMouse = true; });
	inputManager->bindUniqueButtonReleased("RMB", rmb, [this]() { rotateByMouse = false; });
}

void PlayerController::tickGameplay(float deltaSeconds)
{
	Camera& camera = getWorld()->getCamera();
	InputManager* input = getWorld()->getInputManager();

	float powerForward = input->getAxis("moveForward") * 500.0f;
	float powerTurn = input->getAxis("moveRight") * 50.0f;

	vector3 forwardDir = pawnRotation.toDirection();

	linearSpeed *= 0.998f;
	linearSpeed += powerForward * deltaSeconds;
	vector3 linearVelocity = linearSpeed * forwardDir;

	// #todo-game: torque
	float turnRate = powf(pathos::min(1.0f, fabs(linearSpeed) / 500.0f), 3.0f);
	//LOG(LogDebug, "linSpeed=%f turnRate=%f", linearSpeed, turnRate);
	if (linearSpeed > 0.0f) {
		pawnRotation.yaw -= turnRate * powerTurn * deltaSeconds;
	} else if (linearSpeed < 0.0f) {
		pawnRotation.yaw += turnRate * powerTurn * deltaSeconds;
	}

	vector3 pawnLoc = playerPawn->getActorLocation();
	pawnLoc += linearVelocity * deltaSeconds;
	playerPawn->setActorLocation(pawnLoc);
	playerPawn->setActorRotation(pawnRotation);

	const float CAMERA_HEIGHT_OFFSET = 40.0f;
	vector3 cameraLoc = pawnLoc;
	cameraLoc -= forwardDir * 200.0f;
	cameraLoc.y += CAMERA_HEIGHT_OFFSET;
	camera.lookAt(cameraLoc, pawnLoc + vector3(0.0f, CAMERA_HEIGHT_OFFSET, 0.0f), vector3(0.0f, 1.0f, 0.0f));
}

void PlayerController::tickPhotoMode(float deltaSeconds)
{
	Camera& camera = getWorld()->getCamera();
	InputManager* input = getWorld()->getInputManager();

	static int32 prevMouseX = 0;
	static int32 prevMouseY = 0;
	int32 currMouseX = input->getMouseX();
	int32 currMouseY = input->getMouseY();

	// movement per seconds
	const float moveMultiplier = pathos::max(1.0f, input->getAxis("moveFast") * 10.0f);
	const float speedRight = 200.0f * deltaSeconds * moveMultiplier;
	const float speedForward = 200.0f * deltaSeconds * moveMultiplier;
	const float speedUp = 200.0f * deltaSeconds * moveMultiplier;
	const float rotateYaw = 120.0f * deltaSeconds;
	const float rotatePitch = 120.0f * deltaSeconds;

	float deltaRight = input->getAxis("moveRight") * speedRight;
	float deltaForward = input->getAxis("moveForward") * speedForward;
	float deltaUp = input->getAxis("moveUp") * speedUp;
	float rotY = 0.1f * (currMouseX - prevMouseX) * rotateYaw;
	float rotX = 0.1f * (currMouseY - prevMouseY) * rotatePitch;

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
