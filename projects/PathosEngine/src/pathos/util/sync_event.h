#pragma once

#include "badger/types/int_types.h"

#include <mutex>
#include <atomic>
#include <vector>

namespace pathos {

	class SyncEvent {
		friend class Fence;
		
	public:
		SyncEvent();
		~SyncEvent();

		void waitInfinite();
		void wake();
		void close();

	private:
		void* nativeEvent = nullptr;

	};

	class Fence {

	public:
		Fence(uint64 initialValue) {
			value = initialValue;
		}

		// If fence value reaches at targetValue, wake syncEvent.
		inline void setEventOnCompletion(uint64 targetValue, SyncEvent* syncEvent) {
			if (value == targetValue) {
				syncEvent->wake();
			} else {
				syncEvents.push_back(syncEvent);
				syncValues.push_back(targetValue);
			}
		}

		inline void signalValue(uint64 newValue) {
			value = newValue;

			for (size_t i = 0; i < syncEvents.size(); ++i) {
				if (syncValues[i] == newValue) {
					syncEvents[i]->wake();
					syncEvents.erase(syncEvents.begin() + i);
					syncValues.erase(syncValues.begin() + i);
					--i;
				}
			}
		}

		inline uint64 getValue() const { return value; }

	private:
		std::atomic<uint64> value = 0;

		std::vector<SyncEvent*> syncEvents;
		std::vector<uint64> syncValues;

	};

}
