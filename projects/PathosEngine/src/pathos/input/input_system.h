#pragma once

#include "badger/types/int_types.h"
#include "input_constants.h"

#include <list>

namespace pathos {

	class InputManager;

	class InputSystem final {

	public:
		InputSystem();
		~InputSystem();

		InputManager* getDefaultInputManager() const { return defaultInputManager; }

		void tick();

		void processRawKeyDown(uint8 ascii);
		void processRawKeyUp(uint8 ascii);
		void processSpecialKeyDown(InputConstants modifier);
		void processSpecialKeyUp(InputConstants modifier);

	private:
		InputManager* defaultInputManager;
		std::list<InputManager*> inputChain;

	};

}
