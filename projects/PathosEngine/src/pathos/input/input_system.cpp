#include "input_system.h"

#include "pathos/input/input_manager.h"

namespace pathos {

	InputSystem::InputSystem()
	{
		defaultInputManager = new InputManager;
		inputChain.push_back(defaultInputManager);
	}

	InputSystem::~InputSystem()
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			delete manager;
		}
	}

	void InputSystem::tick()
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->updateAxisValue();
		}
	}

	void InputSystem::processRawKeyDown(uint8 ascii)
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->processRawKeyDown(ascii);
		}
	}

	void InputSystem::processRawKeyUp(uint8 ascii)
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->processRawKeyUp(ascii);
		}
	}

}
