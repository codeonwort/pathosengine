#include "input_system.h"

#include "pathos/input/input_manager.h"
#include "pathos/input/xinput_manager.h"

namespace pathos {

	InputSystem::InputSystem() {
		defaultInputManager = new InputManager(this);
		inputChain.push_back(defaultInputManager);

		g_xinputManager = new XInputManager;
		//defaultInputManager->bindXInput(XInputUserIndex::USER0);
	}

	InputSystem::~InputSystem() {
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			delete manager;
		}
	}

	pathos::InputManager* InputSystem::createInputManager() {
		InputManager* im = new InputManager(this);
		inputChain.push_back(im);
		return im;
	}

	void InputSystem::unregisterInputManager(InputManager* inputManager) {
		inputChain.remove(inputManager);
	}

	void InputSystem::tick() {
		g_xinputManager->update();

		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			if (!manager->bActivated) continue;
			manager->tick();
			manager->updateAxisValue();
		}
	}

	void InputSystem::processRawKeyDown(uint8 ascii) {
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			if (!manager->bActivated) continue;
			manager->processRawKeyDown(ascii);
		}
	}

	void InputSystem::processRawKeyUp(uint8 ascii) {
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			if (!manager->bActivated) continue;
			manager->processRawKeyUp(ascii);
		}
	}

	void InputSystem::processSpecialKeyDown(InputConstants specialKey) {
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			if (!manager->bActivated) continue;
			manager->processSpecialKeyDown(specialKey);
		}
	}

	void InputSystem::processSpecialKeyUp(InputConstants specialKey) {
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			if (!manager->bActivated) continue;
			manager->processSpecialKeyUp(specialKey);
		}
	}

	void InputSystem::processButtonDown(InputConstants input) {
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			if (!manager->bActivated) continue;
			manager->processButtonDown(input);
		}
	}

	void InputSystem::processButtonUp(InputConstants input) {
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			if (!manager->bActivated) continue;
			manager->processButtonUp(input);
		}
	}

}
