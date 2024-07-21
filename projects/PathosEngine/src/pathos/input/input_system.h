#pragma once

#include "badger/types/int_types.h"
#include "input_constants.h"

#include <list>
#include <memory>

namespace pathos {

	class InputManager;
	class XInputManager;

	// InputSystem is a bridge between Engine and InputManager.
	// It just delegates all works to input managers.
	class InputSystem final {
		friend class InputManager;

	public:
		InputSystem();
		~InputSystem();

		// CAUTION: Usually each world instance should use its own input manager via World::getInputManager().
		InputManager* getDefaultInputManager() const { return defaultInputManager; }

		// Create an input manager and add to input chain.
		InputManager* createInputManager();

		void tick();

		// #todo-input: Integrate
		void processRawKeyDown(uint8 ascii);
		void processRawKeyUp(uint8 ascii);
		void processSpecialKeyDown(InputConstants modifier);
		void processSpecialKeyUp(InputConstants modifier);
		void processButtonDown(InputConstants input);
		void processButtonUp(InputConstants input);

	private:
		void unregisterInputManager(InputManager* inputManager);

		// - Shared by every aspects of the engine instance.
		// - Can be shared across different worlds, so it's better that each world instance
		//   create its own input manager.
		InputManager* defaultInputManager;
		std::list<InputManager*> inputChain;
	};

}
