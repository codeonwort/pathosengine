#pragma once

#include "badger/system/stopwatch.h"
#include "badger/types/int_types.h"
#include <vector>

namespace pathos {

	struct ScopedCpuCounter : Stopwatch {
		ScopedCpuCounter(const char* inName);
		~ScopedCpuCounter();

		uint32 coreIndex;
		const char* name;
		uint32 frameHandle;
	};

	class CpuProfiler {
		struct ProfileFrame {
			ProfileFrame(const char* inName, uint32 inTab)
				: name(inName)
				, tab(inTab)
				, elapsedMS(-1.0f)
			{
			}
			const char* name;
			uint32 tab;
			float elapsedMS;
		};
		struct ProfilePerCore {
			ProfilePerCore()
				: tab(0)
			{
			}
			void clear() {
				frames.clear();
				tab = 0;
			}
			bool isEmpty() const {
				return frames.size() == 0;
			}
			std::vector<ProfileFrame> frames;
			uint32 tab;
		};

	public:
		static CpuProfiler& getInstance();

		// Called in Engine::tick()
		void clearProfile();
		void collectProfile();

		uint32 pushCounter(uint32 coreIndex, const char* counterName);
		void popCounter(uint32 frameHandle, uint32 coreIndex, float elapsedMS);

	private:
		CpuProfiler() = default;
		~CpuProfiler() = default;

		std::vector<ProfilePerCore> profiles;

	};

}

#define SCOPED_CPU_COUNTER(CounterName) ScopedCpuCounter cpu_counter_##CounterName(#CounterName)
