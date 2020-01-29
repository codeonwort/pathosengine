#include "input_manager.h"

#include "badger/types/string_hash.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	void InputManager::bindButtonPressed(const char* eventName, const ButtonBinding& binding, std::function<void()> handler)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		CHECK(buttonPressedMapping.find(hash) == buttonPressedMapping.end());

		ButtonBinding ib = binding;
		ib.event_name_hash = hash;
		buttonPressedBindings.emplace_back(ib);

		buttonPressedMapping.insert(std::make_pair(hash, handler));
	}

	void InputManager::bindButtonReleased(const char* eventName, const ButtonBinding& binding, std::function<void()> handler)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		CHECK(buttonReleasedMapping.find(hash) == buttonReleasedMapping.end());

		ButtonBinding ib = binding;
		ib.event_name_hash = hash;
		buttonReleasedBindings.emplace_back(ib);

		buttonReleasedMapping.insert(std::make_pair(hash, handler));
	}

	void InputManager::updateAxisValue()
	{
		std::vector<InputConstants> activeKeys;
		activeKeys.reserve(256);

		for (uint32 i = 0; i <= 255; ++i) {
			if (asciiMap[i]) {
				InputConstants ic = asciiToInputConstants[i];
				if (ic != InputConstants::UNDEFINED) {
					activeKeys.push_back(ic);
				}
			}
		}

		for (const AxisBinding& binding : axisBindings) {
			axisMapping[binding.event_name_hash] = 0.0f;

			int32 numKeys = (int32)binding.keys.size();
			for (int32 i = 0; i < numKeys; ++i) {
				if (std::find(activeKeys.begin(), activeKeys.end(), binding.keys[i]) != activeKeys.end()) {
					axisMapping[binding.event_name_hash] = binding.multipliers[i];

					// #todo-input: What if multiple keys are pressed for this event?
					break;
				}
			}
		}
	}

	void InputManager::bindAxis(const char* eventName, const AxisBinding& binding)
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		CHECK(axisMapping.find(hash) == axisMapping.end());

		AxisBinding ib = binding;
		ib.event_name_hash = hash;
		axisBindings.emplace_back(ib);

		axisMapping.insert(std::make_pair(hash, 0.0f));
	}

	float InputManager::getAxis(const char* eventName) const
	{
		uint32 hash = COMPILE_TIME_CRC32_STR(eventName);
		return axisMapping.find(hash)->second;
	}

	void InputManager::processRawKeyDown(uint8 ascii)
	{
		std::vector<uint32> matchingEvents;

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

}
