#pragma once

#include "badger/types/string_hash.h"
#include <string>
#include <vector>

namespace pathos {

	class RenderCommandList;

	struct DebugGroupMarker {

		DebugGroupMarker(RenderCommandList* cmdList, const char* eventName);
		~DebugGroupMarker();

	private:
		RenderCommandList* command_list;

	};

}

namespace pathos {

	// CAUTION: Use only in the render thread.
	// #todo-gpu-counter: Some operations should be atomic if render thread goes multi-threaded.
	struct ScopedGpuCounter {

		static const uint32 MAX_GPU_COUNTERS;

		ScopedGpuCounter(RenderCommandList* cmdList, const char* inCounterName);
		~ScopedGpuCounter();

	private:
		RenderCommandList* command_list;
		std::string counterName;
		GLuint queryObject1, queryObject2; // begin timestamp, end timestamp

	public:
		static void initializeQueryObjectPool(uint32 inMaxGpuCounters = MAX_GPU_COUNTERS);
		static void destroyQueryObjectPool();
		static uint32 flushQueries(RenderCommandList* cmdList, std::vector<std::string>& outCounterNames, std::vector<float>& outElapsedMilliseconds);

		static bool bEnableCounter; // #todo-gpu-counter: Support gpu counter in scene capture. This is a hack.

	private:
		static bool getUnusedQueryObject(const char* inCounterName, GLuint& outQuery1, GLuint& outQuery2);

		static bool poolInitialized;
		static bool canBeginQuery;
		static uint32 maxQueryObjects;
		static uint32 numUsedQueryObjects;
		static std::vector<GLuint> queryObjectPool;
		static std::vector<std::string> queryCounterNames;
	};

}

// Assumes 'RenderCommandList& cmdList' is defined in the caller
#define SCOPED_DRAW_EVENT(EventName) pathos::DebugGroupMarker DebugGroup_##EventName(&cmdList, #EventName);
#define SCOPED_DRAW_EVENT_STRING(EventString) pathos::DebugGroupMarker DebugGroup_##EventName(&cmdList, EventString);

#define SCOPED_GPU_COUNTER(CounterName) pathos::ScopedGpuCounter ScopedGpuCounter_##CounterName(&cmdList, #CounterName);
