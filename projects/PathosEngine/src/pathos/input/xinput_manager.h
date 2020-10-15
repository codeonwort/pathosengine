#pragma once

#include "badger/types/int_types.h"
#include "badger/types/os.h"
#include "pathos/input/input_constants.h"

#include <vector>

namespace pathos {

	enum class XInputUserIndex : uint32
	{
		USER0 = 0,
		USER1 = 1,
		USER2 = 2,
		USER3 = 3,
		MAX_USERS = 4
	};

	struct XInputState
	{
		XInputState()
			: connected(false)
			, packetNumber(0xffffffff)
			, buttonState(0)
			, buttonStatePrev(0)
			, leftTrigger(0.0f)
			, rightTrigger(0.0f)
			, leftThumbX(0.0f)
			, leftThumbY(0.0f)
			, rightThumbX(0.0f)
			, rightThumbY(0.0f)
			, numActiveKeys(0)
			, numPressed(0)
			, numReleased(0)
		{
		}

		bool connected;
		uint32 packetNumber;
		uint32 buttonState;
		uint32 buttonStatePrev;
		float leftTrigger;
		float rightTrigger;
		float leftThumbX;
		float leftThumbY;
		float rightThumbX;
		float rightThumbY;

		uint32 numActiveKeys;
		InputConstants activeKeys[32];
		float activeKeysMultiplier[32];

		uint32 numPressed;
		uint32 numReleased;
		InputConstants buttonsPressed[16];
		InputConstants buttonsReleased[16];
	};

#if PLATFORM_WINDOWS == 0
	class XInputManager
	{
	public:
		static constexpr bool isSupported = false;

		inline void update() {}
		inline bool isConnected(XInputUserIndex userIndex) const { return false; }
		inline void appendActiveKeys(XInputUserIndex userIndex, std::vector<std::pair<InputConstants, float>>& outKeys) {}
		inline void getPressed(XInputUserIndex userIndex, std::vector<InputConstants>& outButtons) {}
		inline void getReleased(XInputUserIndex userIndex, std::vector<InputConstants>& outButtons) {}
	};
#else
	class XInputManager
	{
	public:
		static constexpr bool isSupported = true;
		static constexpr float deadzoneX = 0.2f; // Make configurable if needed
		static constexpr float deadzoneY = 0.2f; // Make configurable if needed

		void update();
		bool isConnected(XInputUserIndex userIndex) const;
		void appendActiveKeys(XInputUserIndex userIndex, std::vector<std::pair<InputConstants, float>>& outKeys);
		void getPressed(XInputUserIndex userIndex, std::vector<InputConstants>& outButtons);
		void getReleased(XInputUserIndex userIndex, std::vector<InputConstants>& outButtons);

	private:
		XInputState states[static_cast<uint32>(XInputUserIndex::MAX_USERS)];
	};
#endif // PLATFORM_WINDOWS

	extern XInputManager* g_xinputManager;
}
