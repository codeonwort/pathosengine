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

	struct GpuCounterContext {
		bool                     bPoolInitialized    = false;
		uint32                   maxQueryObjects     = 0;
		uint32                   numUsedQueryObjects = 0;
		int32                    nested = 0;
		std::vector<GLuint>      queryObjectPool;
		std::vector<std::string> queryCounterNames;
		std::vector<int32>       indentLevels;
	};

	struct GpuCounterResult {
		uint32                   numCounters = 0;
		std::vector<std::string> counterNames;
		std::vector<float>       elapsedMilliseconds;
		std::vector<int32>       indentLevels;
	};

	// CAUTION: Use only in the render thread.
	// #todo-gpu-counter: Some operations should be atomic if render thread goes multi-threaded.
	struct ScopedGpuCounter {
		ScopedGpuCounter(RenderCommandList* cmdList, const char* inCounterName);
		~ScopedGpuCounter();

	private:
		RenderCommandList* command_list;
		std::string counterName;
		GLuint queryObject1, queryObject2; // begin timestamp, end timestamp

	public:
		static void initializeQueryObjectPool(uint32 inMaxGpuCounters = MAX_GPU_COUNTERS);
		static void destroyQueryObjectPool();
		static GpuCounterResult flushQueries(RenderCommandList* cmdList);

	private:
		static bool getUnusedQueryObject(const char* inCounterName, int32 nested, GLuint& outQuery1, GLuint& outQuery2);

		static const uint32 MAX_GPU_COUNTERS;
		static GpuCounterContext context;
	};

}

// Assumes 'RenderCommandList& cmdList' is defined in the caller
#define SCOPED_DRAW_EVENT(EventName) pathos::DebugGroupMarker DebugGroup_##EventName(&cmdList, #EventName);

#define SCOPED_DRAW_EVENT_STRING_INTERNAL2(X, Y, Z) X ## Y ## Z
#define SCOPED_DRAW_EVENT_STRING_INTERNAL(EventString, Line) SCOPED_DRAW_EVENT_STRING_INTERNAL2(pathos::DebugGroupMarker DebugGroup, Line, (&cmdList, EventString));
#define SCOPED_DRAW_EVENT_STRING(EventString) SCOPED_DRAW_EVENT_STRING_INTERNAL(EventString, __LINE__)

#define SCOPED_GPU_COUNTER(CounterName) pathos::ScopedGpuCounter ScopedGpuCounter_##CounterName(&cmdList, #CounterName);
