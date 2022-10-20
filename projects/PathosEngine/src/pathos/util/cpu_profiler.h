#pragma once

#include "badger/system/stopwatch.h"
#include "badger/types/int_types.h"
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace pathos {

	struct ScopedCpuCounter {
		ScopedCpuCounter(const char* inName);
		~ScopedCpuCounter();

		uint32 threadId;
		// #todo-stat: Assumes string literal
		const char* name;
		uint32 itemHandle;

		// #todo-cpu: Temp value due to temp periodic purge
		uint32 purge_milestone;
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

	// It turned out that per-core profile is a bad idea. Let's go for per-thread profile.
	struct ProfilePerThread {
		ProfilePerThread(uint32 inThreadId, const char* inDebugName)
			: threadId(inThreadId)
			, threadName(inDebugName)
			, currentTab(0)
		{
		}
		ProfilePerThread(uint32 inThreadId, std::string&& inDebugName)
			: threadId(inThreadId)
			, threadName(inDebugName)
			, currentTab(0)
		{
		}

		void clearItems() {
			items.clear();
			currentTab = 0;
		}
		bool isEmpty() const {
			return items.size() == 0;
		}

		uint32 threadId;
		std::string threadName;

		std::vector<ProfileItem> items;
		uint32 currentTab;

	// DO NOT USE. For unordered_map only.
	public:
		ProfilePerThread()
			: threadId(0xffffffff)
			, threadName("")
			, currentTab(0)
		{
		}
	};

	struct ProfileCheckpoint {
		float startTime;       // in seconds
		float endTime = -1.0f; // in seconds
		uint32 frameCounter;
	};

	class CpuProfiler {

	public:
		static CpuProfiler& getInstance();

		void initialize();
		void registerCurrentThread(const char* inDebugName);
		void registerThread(uint32 threadId, const char* inDebugName);

		void beginCheckpoint(uint32 frameCounter);
		void finishCheckpoint();

		uint32 beginItem(uint32 threadId, const char* counterName);
		void finishItem(uint32 frameHandle, uint32 threadId);

		void getLastFrameSnapshot(uint32 threadID, std::vector<ProfileItem>& outSnapshot);

		// #todo-cpu: Temp
		std::atomic<uint32> purge_milestone;

	private:
		CpuProfiler() = default;
		~CpuProfiler() = default;

		void purgeEverything();

		// bChromeTracingFormat: Dump as plain text or JSON that chrome://tracing and edge://tracing recognizes.
		// See https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU
		void dumpCPUProfile(int32 numFramesToDump, bool bChromeTracingFormat);

		float getGlobalClockTime();

		std::unordered_map<uint32, ProfilePerThread> profiles; // Map thread id to profile
		std::mutex profilesMutex;

		std::vector<ProfileCheckpoint> checkpoints; // #todo-cpu: Switch to typed ring buffers
		std::mutex checkpointMutex;

		std::vector<Stopwatch> globalClocks; // Per-core so that no mutex is necessary
	};

}

#define SCOPED_CPU_COUNTER(CounterName) ScopedCpuCounter cpu_counter_##CounterName(#CounterName)
