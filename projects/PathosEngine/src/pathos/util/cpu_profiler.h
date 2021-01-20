#pragma once

#include "badger/system/stopwatch.h"
#include "badger/types/int_types.h"
#include <vector>

namespace pathos {

	struct ScopedCpuCounter {
		ScopedCpuCounter(const char* inName);
		~ScopedCpuCounter();

		uint32 coreIndex;
		const char* name;
		uint32 itemHandle;
	};

	struct ProfileItem {
		ProfileItem(const char* inName, uint32 inTab, float inStartTime)
			: name(inName)
			, tab(inTab)
			, startTime(inStartTime)
			, endTime(-1.0f)
			, elapsedMS(-1.0f)
		{
		}
		const char* name;
		uint32 tab;
		float startTime; // in seconds
		float endTime;   // in seconds
		float elapsedMS; // in milliseconds
	};

	// For main threads, top-level items will cover a single frame entirely.
	// For background threads, top-level items are desychronized with frame boundaries, even might linger on several frames.
	struct ProfilePerCore {
		ProfilePerCore()
			: currentTab(0)
		{
		}
		void clear() {
			items.clear();
			currentTab = 0;
		}
		bool isEmpty() const {
			return items.size() == 0;
		}
		std::vector<ProfileItem> items;
		uint32 currentTab;
	};

	struct ProfileCheckpoint {
		float startTime; // in seconds
		float endTime;   // in seconds
		uint32 logicalCoreIndex;
		uint32 frameCounter;
	};

	class CpuProfiler {

	public:
		static CpuProfiler& getInstance();

		void initialize();

		void beginCheckpoint(uint32 frameCounter);
		void finishCheckpoint();

		uint32 beginItem(uint32 coreIndex, const char* counterName);
		void finishItem(uint32 frameHandle, uint32 coreIndex);

	private:
		CpuProfiler() = default;
		~CpuProfiler() = default;

		void purgeEverything();

		// Per-thread so that no mutex is necessary
		// #todo-cpu: Switch to typed ring buffers
		std::vector<ProfilePerCore> profiles;
		std::vector<ProfileCheckpoint> checkpoints;
		std::vector<Stopwatch> globalClocks;

		// CAUTION: do not place anything thread-unsafe here

	};

}

#define SCOPED_CPU_COUNTER(CounterName) ScopedCpuCounter cpu_counter_##CounterName(#CounterName)
