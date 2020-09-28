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

	public:
		InputSystem();
		~InputSystem();

		InputManager* getDefaultInputManager() const { return defaultInputManager; }

		void tick();

		// #todo-input: Integrate
		void processRawKeyDown(uint8 ascii);
		void processRawKeyUp(uint8 ascii);
		void processSpecialKeyDown(InputConstants modifier);
		void processSpecialKeyUp(InputConstants modifier);
		void processButtonDown(InputConstants input);
		void processButtonUp(InputConstants input);

	private:
		InputManager* defaultInputManager;
		std::list<InputManager*> inputChain;
	};

}
