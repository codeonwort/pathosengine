#include "xinput_manager.h"
#include "badger/math/minmax.h"
#include "badger/assertion/assertion.h"
#include "pathos/input/input_constants.h"

namespace pathos {
	XInputManager* g_xinputManager = nullptr;
}

#if PLATFORM_WINDOWS

#include <Windows.h>
#include <winerror.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

namespace pathos {

	const uint32 bitmasks[] = {
		XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT,
		XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK,
		XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
		XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y };
	const InputConstants inputConsts[] = {
		InputConstants::XBOXONE_DPAD_UP, InputConstants::XBOXONE_DPAD_DOWN, InputConstants::XBOXONE_DPAD_LEFT, InputConstants::XBOXONE_DPAD_RIGHT,
		InputConstants::XBOXONE_START, InputConstants::XBOXONE_BACK,
		InputConstants::XBOXONE_LT, InputConstants::XBOXONE_RT, InputConstants::XBOXONE_LB, InputConstants::XBOXONE_RB,
		InputConstants::XBOXONE_A, InputConstants::XBOXONE_B, InputConstants::XBOXONE_X, InputConstants::XBOXONE_Y };
	const uint32 N_XBOX_PAD_BUTTONS = sizeof(bitmasks) / sizeof(bitmasks[0]);
	static_assert(N_XBOX_PAD_BUTTONS == sizeof(inputConsts) / sizeof(inputConsts[0]), "Invalid mapping");

	void XInputManager::update() {
		for (uint32 userIndex = 0; userIndex < (uint32)XInputUserIndex::MAX_USERS; ++userIndex) {
			XINPUT_STATE rawState;
			DWORD retCode = XInputGetState((DWORD)userIndex, &rawState);

			XInputState& state = states[userIndex];
			if (retCode == ERROR_SUCCESS) {
				state.connected = true;
				if (rawState.dwPacketNumber == state.packetNumber) {
					continue;
				} else {
					// button
					state.packetNumber = rawState.dwPacketNumber;
					state.buttonStatePrev = state.buttonState;
					state.buttonState = (uint32)rawState.Gamepad.wButtons;
					
					state.numActiveKeys = 0;
					state.numPressed = 0;
					state.numReleased = 0;

					for (uint32 i = 0; i < N_XBOX_PAD_BUTTONS; ++i) {
						uint32 currentBit = state.buttonState & bitmasks[i];

						if (currentBit != 0) {
							state.activeKeys[state.numActiveKeys] = inputConsts[i];
							state.activeKeysMultiplier[state.numActiveKeys] = 1.0f;
							state.numActiveKeys += 1;
							if ((state.buttonStatePrev & bitmasks[i]) == 0) {
								// pressed in this tick
								state.buttonsPressed[state.numPressed++] = inputConsts[i];
							}
						} else {
							if ((state.buttonStatePrev & bitmasks[i]) != 0) {
								// released in this tick
								state.buttonsReleased[state.numReleased++] = inputConsts[i];
							}
						}
					}

					// axis
					state.leftTrigger = (float)rawState.Gamepad.bLeftTrigger / 255.0f;
					state.rightTrigger = (float)rawState.Gamepad.bRightTrigger / 255.0f;
					state.leftThumbX = badger::clamp(-1.0f, (float)rawState.Gamepad.sThumbLX / 32767.0f, 1.0f);
					state.leftThumbY = badger::clamp(-1.0f, (float)rawState.Gamepad.sThumbLY / 32767.0f, 1.0f);
					state.rightThumbX = badger::clamp(-1.0f, (float)rawState.Gamepad.sThumbRX / 32767.0f, 1.0f);
					state.rightThumbY = badger::clamp(-1.0f, (float)rawState.Gamepad.sThumbRY / 32767.0f, 1.0f);

					if (fabs(state.leftThumbX) <= deadzoneX) state.leftThumbX = 0.0f;
					if (fabs(state.leftThumbY) <= deadzoneY) state.leftThumbY = 0.0f;
					if (fabs(state.rightThumbX) <= deadzoneX) state.rightThumbX = 0.0f;
					if (fabs(state.rightThumbY) <= deadzoneY) state.rightThumbY = 0.0f;

					if (state.leftTrigger != 0.0f) {
						state.activeKeys[state.numActiveKeys] = InputConstants::XBOXONE_LEFT_TRIGGER;
						state.activeKeysMultiplier[state.numActiveKeys] = state.leftTrigger;
						state.numActiveKeys += 1;
					}
					if (state.rightTrigger != 0.0f) {
						state.activeKeys[state.numActiveKeys] = InputConstants::XBOXONE_RIGHT_TRIGGER;
						state.activeKeysMultiplier[state.numActiveKeys] = state.rightTrigger;
						state.numActiveKeys += 1;
					}
					if (state.leftThumbX != 0.0f) {
						state.activeKeys[state.numActiveKeys] = InputConstants::XBOXONE_LEFT_THUMB_X;
						state.activeKeysMultiplier[state.numActiveKeys] = state.leftThumbX;
						state.numActiveKeys += 1;
					}
					if (state.leftThumbY != 0.0f) {
						state.activeKeys[state.numActiveKeys] = InputConstants::XBOXONE_LEFT_THUMB_Y;
						state.activeKeysMultiplier[state.numActiveKeys] = state.leftThumbY;
						state.numActiveKeys += 1;
					}
					if (state.rightThumbX != 0.0f) {
						state.activeKeys[state.numActiveKeys] = InputConstants::XBOXONE_RIGHT_THUMB_X;
						state.activeKeysMultiplier[state.numActiveKeys] = state.rightThumbX;
						state.numActiveKeys += 1;
					}
					if (state.rightThumbY != 0.0f) {
						state.activeKeys[state.numActiveKeys] = InputConstants::XBOXONE_RIGHT_THUMB_Y;
						state.activeKeysMultiplier[state.numActiveKeys] = state.rightThumbY;
						state.numActiveKeys += 1;
					}
				}
			} else if (retCode == ERROR_DEVICE_NOT_CONNECTED) {
				states[userIndex] = XInputState();
			} else {
				CHECK_NO_ENTRY();
			}
		}
	}

	bool XInputManager::isConnected(XInputUserIndex userIndex) const {
		if ((uint32)userIndex >= (uint32)XInputUserIndex::MAX_USERS) {
			return false;
		}
		return states[(uint32)userIndex].connected;
	}

	void XInputManager::appendActiveKeys(XInputUserIndex userIndex, std::vector<std::pair<InputConstants, float>>& outKeys) {
		if (isConnected(userIndex)) {
			XInputState& state = states[(uint32)userIndex];
			for (uint32 i = 0; i < state.numActiveKeys; ++i) {
				outKeys.push_back({ state.activeKeys[i], state.activeKeysMultiplier[i] });
			}
		}
	}

	void XInputManager::getPressed(XInputUserIndex userIndex, std::vector<InputConstants>& outButtons) {
		if (isConnected(userIndex)) {
			XInputState& state = states[(uint32)userIndex];
			outButtons.resize(state.numPressed);
			for (uint32 i = 0; i < state.numPressed; ++i) {
				outButtons[i] = state.buttonsPressed[i];
			}
		}
	}

	void XInputManager::getReleased(XInputUserIndex userIndex, std::vector<InputConstants>& outButtons) {
		if (isConnected(userIndex)) {
			XInputState& state = states[(uint32)userIndex];
			outButtons.resize(state.numReleased);
			for (uint32 i = 0; i < state.numReleased; ++i) {
				outButtons[i] = state.buttonsReleased[i];
			}
		}
	}

}

#endif // PLATFORM_WINDOWS
