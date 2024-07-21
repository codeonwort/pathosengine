#pragma once

#include "badger/types/int_types.h"
#include "pathos/input/input_constants.h"
#include "pathos/input/xinput_manager.h"

#include <functional>
#include <vector>
#include <map>

namespace pathos {

	//struct InputKey {
	//	InputConstants keyCode;
	//
	//	// Special states
	//	uint8 capsLock : 1;
	//	uint8 ctrl     : 1;
	//	uint8 alt      : 1;
	//	uint8 shift    : 1;
	//};

	struct ButtonBinding {
		friend class InputManager;
		
	public:
		ButtonBinding() {}
		ButtonBinding(std::initializer_list<InputConstants> inKeys) { keys = inKeys; }

		void addInput(InputConstants key) {
			keys.push_back(key);
		}

		bool contains(InputConstants key) const {
			return std::find(keys.begin(), keys.end(), key) != keys.end();
		}

	private:
		uint32 event_name_hash = 0xffffffff;
		std::vector<InputConstants> keys;
		bool bPressFired = false;

	};

	struct AxisBinding {
		friend class InputManager;

	public:
		AxisBinding() {}

		void addInput(InputConstants key, float multiplier) {
			keys.push_back(key);
			multipliers.push_back(multiplier);
		}

		bool contains(InputConstants key) const {
			return std::find(keys.begin(), keys.end(), key) != keys.end();
		}

	private:
		uint32 event_name_hash = 0xffffffff;
		std::vector<InputConstants> keys;
		std::vector<float> multipliers;

	};

	// - Handles buttonPressed, buttonReleased, and axis input types.
	// - Each event name in an input type can have at most one binding and one handler.
	// - Event names for each input type use different namespaces,
	//   so the same event name can be used for e.g., a buttonPressed and a buttonReleased events.
	class InputManager {
		friend class InputSystem;
		friend class XInputManager;

	public:
		~InputManager();

		// Returns true if a buttonPressed binding already exists for the given eventName, false otherwise.
		bool hasButtonPressed(const char* eventName) const;
		// Returns true if a buttonReleased binding already exists for the given eventName, false otherwise.
		bool hasButtonReleased(const char* eventName) const;
		// Returns true if an axis binding already exists for the given eventName, false otherwise.
		bool hasAxis(const char* eventName) const;

		// Register a buttonPressed binding with the given 'eventName'. Returns true if successful, false if a binding for 'eventName' already exists.
		bool bindButtonPressed(const char* eventName, const ButtonBinding& binding, std::function<void()> handler);
		// Register a buttonReleased binding with the given 'eventName'. Returns true if successful, false if a binding for 'eventName' already exists.
		bool bindButtonReleased(const char* eventName, const ButtonBinding& binding, std::function<void()> handler);
		// Register an axis binding with the given 'eventName'. Returns true if successful, false if a binding for 'eventName' already exists.
		bool bindAxis(const char* eventName, const AxisBinding& binding);

		// Unbind an existing buttonPressed binding for 'eventName'. Returns true if there was a binding.
		bool unbindButtonPressed(const char* eventName);
		// Unbind an existing buttonReleased binding for 'eventName'. Returns true if there was a binding.
		bool unbindButtonReleased(const char* eventName);
		// Unbind an existing axis binding for 'eventName'. Returns true if there was a binding.
		bool unbindAxis(const char* eventName);

		void activate();
		// Deactivated input manager does not fire events until activated again.
		void deactivate(bool clearKeyStates = true);

		void tick();
		void updateAxisValue();

		float getAxis(const char* eventName) const;

		inline int32 getMouseX() const { return mouseX; }
		inline int32 getMouseY() const { return mouseY; }

		// - Delegate all input from a xinput device to this input manager.
		// - Multiple input managers might be connected to the same xinput device.
		// - To disconnect, pass XInputUserIndex::MAX_USERS as an argument.
		void bindXInput(XInputUserIndex userIndex);

		void copyKeyStateFrom(InputManager* another);

	private:
		InputManager(InputSystem* inOwner);

		// #todo-input: Integrate
		void processRawKeyDown(uint8 ascii);
		void processRawKeyUp(uint8 ascii);
		void processSpecialKeyDown(InputConstants specialKey);
		void processSpecialKeyUp(InputConstants specialKey);
		void processButtonDown(InputConstants input);
		void processButtonUp(InputConstants input);

	private:
		bool bActivated = true;
		InputSystem* owner = nullptr;

		bool asciiMap[256] = { false, };
		bool isShiftActive = false;
		bool isCtrlActive = false;
		bool isAltActive = false;
		int32 mouseX = 0;
		int32 mouseY = 0;

		std::map<uint32, std::function<void()>> buttonPressedMapping;
		std::map<uint32, std::function<void()>> buttonReleasedMapping;
		std::map<uint32, float> axisMapping;

		std::vector<ButtonBinding> buttonPressedBindings;
		std::vector<ButtonBinding> buttonReleasedBindings;
		std::vector<AxisBinding> axisBindings;

		XInputUserIndex xinputUserIndex = XInputUserIndex::MAX_USERS;

	};

}
