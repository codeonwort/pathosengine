#include "input_manager.h"

#include "badger/system/platform.h"
#include "badger/types/string_hash.h"
#include "badger/assertion/assertion.h"

#if PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
#endif

namespace pathos {

	bool InputManager::hasButtonPressed(const char* eventName) const
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		return (buttonPressedMapping.find(hash) != buttonPressedMapping.end());
	}

	bool InputManager::hasButtonReleased(const char* eventName) const
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		return (buttonReleasedMapping.find(hash) != buttonReleasedMapping.end());
	}

	bool InputManager::hasAxis(const char* eventName) const
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		return (axisMapping.find(hash) != axisMapping.end());
	}

	bool InputManager::bindButtonPressed(const char* eventName, const ButtonBinding& binding, std::function<void()> handler)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		if (buttonPressedMapping.find(hash) == buttonPressedMapping.end()) {
			ButtonBinding ib = binding;
			ib.event_name_hash = hash;
			buttonPressedBindings.emplace_back(ib);
			buttonPressedMapping.insert(std::make_pair(hash, handler));
			return true;
		}
		return false;
	}

	bool InputManager::bindButtonReleased(const char* eventName, const ButtonBinding& binding, std::function<void()> handler)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		if (buttonReleasedMapping.find(hash) == buttonReleasedMapping.end()) {
			ButtonBinding ib = binding;
			ib.event_name_hash = hash;
			buttonReleasedBindings.emplace_back(ib);
			buttonReleasedMapping.insert(std::make_pair(hash, handler));
			return true;
		}
		return false;
	}

	bool InputManager::bindAxis(const char* eventName, const AxisBinding& binding)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		if (axisMapping.find(hash) == axisMapping.end()) {
			AxisBinding ib = binding;
			ib.event_name_hash = hash;
			axisBindings.emplace_back(ib);
			axisMapping.insert(std::make_pair(hash, 0.0f));
			return true;
		}
		return false;
	}

	void InputManager::tick() {
		// Process xinput button events
		{
			std::vector<InputConstants> buttons;
			g_xinputManager->getPressed(xinputUserIndex, buttons);
			for (InputConstants button : buttons) {
				processButtonDown(button);
			}
			g_xinputManager->getReleased(xinputUserIndex, buttons);
			for (InputConstants button : buttons) {
				processButtonUp(button);
			}
		}

#if PLATFORM_WINDOWS
		POINT mousePos;
		::GetCursorPos(&mousePos);

		mouseX = static_cast<int32>(mousePos.x);
		mouseY = static_cast<int32>(mousePos.y);
#else
	#error "Not implemented for non-Windows OS"
#endif
	}

	void InputManager::updateAxisValue()
	{
		std::vector<std::pair<InputConstants,float>> activeKeys;
		activeKeys.reserve(256);

		g_xinputManager->appendActiveKeys(xinputUserIndex, activeKeys);

		for (uint32 i = 0; i <= 255; ++i) {
			if (asciiMap[i]) {
				InputConstants ic = asciiToInputConstants[i];
				if (ic != InputConstants::UNDEFINED) {
					activeKeys.push_back({ ic, 1.0f });
				}
			}
		}

		if (isShiftActive) activeKeys.push_back({ InputConstants::SHIFT, 1.0f });
		if (isCtrlActive) activeKeys.push_back({ InputConstants::CTRL, 1.0f });
		if (isAltActive) activeKeys.push_back({ InputConstants::ALT, 1.0f });

		for (const AxisBinding& binding : axisBindings) {
			axisMapping[binding.event_name_hash] = 0.0f;

			int32 numKeys = (int32)binding.keys.size();
			for (int32 i = 0; i < numKeys; ++i) {
				auto it = std::find_if(activeKeys.begin(), activeKeys.end(), [&](const auto& activeKey) { return activeKey.first == binding.keys[i]; });
				if (it != activeKeys.end()) {
					axisMapping[binding.event_name_hash] = it->second * binding.multipliers[i];

					// #todo-input: What if this binding has multiple keys and they are pressed at the same time?
					break;
				}
			}
		}
	}

	bool InputManager::unbindButtonPressed(const char* eventName)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		size_t numRemoved = buttonPressedMapping.erase(hash);
		return (bool)numRemoved;
	}

	bool InputManager::unbindButtonReleased(const char* eventName)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		size_t numRemoved = buttonReleasedMapping.erase(hash);
		return (bool)numRemoved;
	}

	bool InputManager::unbindAxis(const char* eventName)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		size_t numRemoved = axisMapping.erase(hash);
		return (bool)numRemoved;
	}

	float InputManager::getAxis(const char* eventName) const
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		return axisMapping.find(hash)->second;
	}

	void InputManager::bindXInput(XInputUserIndex userIndex)
	{
		xinputUserIndex = userIndex;
	}

	void InputManager::processRawKeyDown(uint8 ascii)
	{
		std::vector<uint32> matchingEvents;

		if ('A' <= ascii && ascii <= 'Z') {
			ascii = 'a' + (ascii - 'A');
		}

		if (asciiMap[ascii] == false) {
			asciiMap[ascii] = true;

			for (const ButtonBinding& binding : buttonPressedBindings) {
				InputConstants ic = asciiToInputConstants[ascii];
				if (ic != InputConstants::UNDEFINED && binding.contains(ic)) {
					matchingEvents.push_back(binding.event_name_hash);
				}
			}

			for (uint32 event : matchingEvents) {
				auto it = buttonPressedMapping.find(event);
				CHECK(it != buttonPressedMapping.end());

				it->second();
			}
		}
	}

	void InputManager::processRawKeyUp(uint8 ascii)
	{
		std::vector<uint32> matchingEvents;

		if ('A' <= ascii && ascii <= 'Z') {
			ascii = 'a' + (ascii - 'A');
		}

		asciiMap[ascii] = false;

		for (const ButtonBinding& binding : buttonReleasedBindings) {
			InputConstants ic = asciiToInputConstants[ascii];
			if (ic != InputConstants::UNDEFINED && binding.contains(ic)) {
				matchingEvents.push_back(binding.event_name_hash);
			}
		}

		for (uint32 event : matchingEvents) {
			auto it = buttonReleasedMapping.find(event);
			CHECK(it != buttonReleasedMapping.end());

			it->second();
		}
	}

	void InputManager::processSpecialKeyDown(InputConstants specialKey)
	{
		std::vector<uint32> matchingEvents;

		if (specialKey == InputConstants::SHIFT) isShiftActive = true;
		else if (specialKey == InputConstants::CTRL) isCtrlActive = true;
		else if (specialKey == InputConstants::ALT) isAltActive = true;

		// #todo-input: Redundant; same as processRawKeyDown
		for (const ButtonBinding& binding : buttonPressedBindings) {
			if (binding.contains(specialKey)) {
				matchingEvents.push_back(binding.event_name_hash);
			}
		}

		for (uint32 event : matchingEvents) {
			auto it = buttonPressedMapping.find(event);
			CHECK(it != buttonPressedMapping.end());

			it->second();
		}
	}

	void InputManager::processSpecialKeyUp(InputConstants specialKey)
	{
		std::vector<uint32> matchingEvents;

		if (specialKey == InputConstants::SHIFT) isShiftActive = false;
		else if (specialKey == InputConstants::CTRL) isCtrlActive = false;
		else if (specialKey == InputConstants::ALT) isAltActive = false;

		for (const ButtonBinding& binding : buttonReleasedBindings) {
			if (binding.contains(specialKey)) {
				matchingEvents.push_back(binding.event_name_hash);
			}
		}

		for (uint32 event : matchingEvents) {
			auto it = buttonReleasedMapping.find(event);
			CHECK(it != buttonReleasedMapping.end());

			it->second();
		}
	}

	void InputManager::processButtonDown(InputConstants input)
	{
		std::vector<uint32> matchingEvents;

		// #todo-input: Redundant; same as processRawKeyDown
		for (const ButtonBinding& binding : buttonPressedBindings) {
			if (binding.contains(input)) {
				matchingEvents.push_back(binding.event_name_hash);
			}
		}

		for (uint32 event : matchingEvents) {
			auto it = buttonPressedMapping.find(event);
			CHECK(it != buttonPressedMapping.end());

			it->second();
		}
	}

	void InputManager::processButtonUp(InputConstants input)
	{
		std::vector<uint32> matchingEvents;

		for (const ButtonBinding& binding : buttonReleasedBindings) {
			if (binding.contains(input)) {
				matchingEvents.push_back(binding.event_name_hash);
			}
		}

		for (uint32 event : matchingEvents) {
			auto it = buttonReleasedMapping.find(event);
			CHECK(it != buttonReleasedMapping.end());

			it->second();
		}
	}

}
