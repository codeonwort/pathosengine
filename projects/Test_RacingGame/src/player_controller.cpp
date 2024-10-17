#include "player_controller.h"

#include "pathos/engine.h"
#include "pathos/scene/world.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/landscape_actor.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

#include "badger/math/minmax.h"

#define MOVE_FAST_FACTOR 100.0f

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
	inputManager->bindAxis("moveForward", moveForward);
	inputManager->bindAxis("moveRight", moveRight);
	inputManager->bindAxis("moveUp", moveUp);
	inputManager->bindAxis("moveFast", moveFast);
	inputManager->bindAxis("rotateYaw", rotateYaw);
	inputManager->bindAxis("rotatePitch", rotatePitch);
	// 'reload_scene' command respawns the player controller, so unbind previous bindings by previous player.
	// Axis bindings are OK to fail to renew, as they are invariant.
	inputManager->unbindButtonPressed("RMB");
	inputManager->unbindButtonReleased("RMB");
	inputManager->bindButtonPressed("RMB", rmb, [this]() { rotateByMouse = true; });
	inputManager->bindButtonReleased("RMB", rmb, [this]() { rotateByMouse = false; });
}

void PlayerController::tickGameplay(float deltaSeconds)
{
	Camera& camera = getWorld()->getCamera();
	InputManager* input = getWorld()->getInputManager();

	float powerForward = input->getAxis("moveForward") * 50.0f;
	float powerTurn = input->getAxis("moveRight") * 50.0f;

	vector3 forwardDir = pawnRotation.toDirection();

	linearSpeed *= 0.998f;
	linearSpeed += powerForward * deltaSeconds;
	vector3 linearVelocity = linearSpeed * forwardDir;

	// #todo-game: Simulate torque
	float turnRate = powf(badger::min(1.0f, fabs(linearSpeed) / 50.0f), 3.0f);
	//LOG(LogDebug, "linSpeed=%f turnRate=%f", linearSpeed, turnRate);
	if (linearSpeed > 0.0f) {
		pawnRotation.yaw -= turnRate * powerTurn * deltaSeconds;
	} else if (linearSpeed < 0.0f) {
		pawnRotation.yaw += turnRate * powerTurn * deltaSeconds;
	}

	vector3 pawnLoc = playerPawn->getActorLocation();
	pawnLoc += linearVelocity * deltaSeconds;
	if (landscape != nullptr) {
		pawnLoc.y = landscape->getLandscapeY(pawnLoc.x, pawnLoc.z);
	}
	playerPawn->setActorLocation(pawnLoc);
	
	// Because SportsCar asset is facing +X...
	Rotator pawnMeshRotation = pawnRotation;
	pawnMeshRotation.yaw += 90.0f;
	playerPawn->setActorRotation(pawnMeshRotation);

	vector3 cameraLoc = pawnLoc;
	cameraLoc -= forwardDir * cameraForwardOffset;
	cameraLoc.y += cameraHeightOffset;
	camera.lookAt(cameraLoc, pawnLoc + vector3(0.0f, cameraHeightOffset, 0.0f), vector3(0.0f, 1.0f, 0.0f));
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
	const float moveMultiplier = badger::max(1.0f, input->getAxis("moveFast") * MOVE_FAST_FACTOR);
	const float speedRight   = 2.0f * deltaSeconds * moveMultiplier;
	const float speedForward = 2.0f * deltaSeconds * moveMultiplier;
	const float speedUp      = 2.0f * deltaSeconds * moveMultiplier;
	const float rotateYaw    = 120.0f * deltaSeconds;
	const float rotatePitch  = 120.0f * deltaSeconds;

	float deltaRight   = input->getAxis("moveRight") * speedRight;
	float deltaForward = input->getAxis("moveForward") * speedForward;
	float deltaUp      = input->getAxis("moveUp") * speedUp;
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
