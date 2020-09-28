#include "input_system.h"

#include "pathos/input/input_manager.h"
#include "pathos/input/xinput_manager.h"

namespace pathos {

	InputSystem::InputSystem()
	{
		defaultInputManager = new InputManager;
		inputChain.push_back(defaultInputManager);

		g_xinputManager = new XInputManager;
		defaultInputManager->bindXInput(XInputUserIndex::USER0);
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
		g_xinputManager->update();

		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->tick();
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

	void InputSystem::processSpecialKeyDown(InputConstants specialKey)
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->processSpecialKeyDown(specialKey);
		}
	}

	void InputSystem::processSpecialKeyUp(InputConstants specialKey)
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->processSpecialKeyUp(specialKey);
		}
	}

	void InputSystem::processButtonDown(InputConstants input)
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->processButtonDown(input);
		}
	}

	void InputSystem::processButtonUp(InputConstants input)
	{
		for (auto it = inputChain.begin(); it != inputChain.end(); ++it) {
			InputManager* manager = *it;
			manager->processButtonUp(input);
		}
	}

}
