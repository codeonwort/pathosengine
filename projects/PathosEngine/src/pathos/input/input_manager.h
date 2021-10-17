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

	class InputManager {
		friend class InputSystem;
		friend class XInputManager;

	public:
		bool hasButtonPressed(const char* eventName) const;
		bool hasButtonReleased(const char* eventName) const;
		bool hasAxis(const char* eventName) const;

		void bindButtonPressed(const char* eventName, const ButtonBinding& binding, std::function<void()> handler);
		void bindButtonReleased(const char* eventName, const ButtonBinding& binding, std::function<void()> handler);
		void bindAxis(const char* eventName, const AxisBinding& binding);

		void bindUniqueButtonPressed(const char* eventName, const ButtonBinding& binding, std::function<void()> handler);
		void bindUniqueButtonReleased(const char* eventName, const ButtonBinding& binding, std::function<void()> handler);
		void bindUniqueAxis(const char* eventName, const AxisBinding& binding);

		void tick();
		void updateAxisValue();
		float getAxis(const char* eventName) const;

		inline int32 getMouseX() const { return mouseX; }
		inline int32 getMouseY() const { return mouseY; }

		void bindXInput(XInputUserIndex userIndex);

	private:
		InputManager() = default;

		// #todo-input: Integrate
		void processRawKeyDown(uint8 ascii);
		void processRawKeyUp(uint8 ascii);
		void processSpecialKeyDown(InputConstants specialKey);
		void processSpecialKeyUp(InputConstants specialKey);
		void processButtonDown(InputConstants input);
		void processButtonUp(InputConstants input);

	private:
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
