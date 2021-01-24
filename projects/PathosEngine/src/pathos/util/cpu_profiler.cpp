#include "cpu_profiler.h"
#include "log.h"

#include "badger/assertion/assertion.h"
#include "badger/thread/cpu.h"

#include <sstream>
#include <utility>

namespace pathos {

	// #todo-cpu: Cleanup everything periodically for now
	static constexpr size_t PURGE_BETWEEN_CHECKPOINTS = 256;
	static constexpr size_t PURGE_ITEMS_AFTER = 512;

	ScopedCpuCounter::ScopedCpuCounter(const char* inName)
		: name(inName)
	{
		CHECK(inName != nullptr);
		threadId = CPU::getCurrentThreadId();
		itemHandle = CpuProfiler::getInstance().beginItem(threadId, name);

		// #todo-cpu: Temp logic for periodic purge
		purge_milestone = CpuProfiler::getInstance().purge_milestone.load();
	}

	ScopedCpuCounter::~ScopedCpuCounter() {
		// #todo-cpu: Temp logic for periodic purge
		const uint32 currentMilestone = CpuProfiler::getInstance().purge_milestone.load();
		if (currentMilestone > purge_milestone) {
			return;
		}

		CpuProfiler::getInstance().finishItem(itemHandle, threadId);
	}

	//////////////////////////////////////////////////////////////////////////

	pathos::CpuProfiler& CpuProfiler::getInstance() {
		static CpuProfiler instance;
		return instance;
	}

	void CpuProfiler::initialize() {
		const uint32 nCores = CPU::getTotalLogicalCoreCount();
		CHECKF(nCores != 0, "Couldn't get total core count");

		globalClocks.resize(nCores);
		for (uint32 i = 0; i < nCores; ++i) {
			globalClocks[i].start();
		}

		purge_milestone.store(0);
	}

	void CpuProfiler::registerCurrentThread(const char* inDebugName) {
		const uint32 threadId = CPU::getCurrentThreadId();
		registerThread(threadId, inDebugName);
	}
	void CpuProfiler::registerThread(uint32 threadId, const char* inDebugName) {
		CHECKF(profiles.find(threadId) == profiles.end(), "Current thread is already registered");
		profiles.insert(std::pair<uint32, ProfilePerThread>(threadId, ProfilePerThread(threadId, inDebugName)));
	}

	void CpuProfiler::beginCheckpoint(uint32 frameCounter)
	{
		const uint32 coreIndex = CPU::getCurrentLogicalCoreIndex();

		ProfileCheckpoint cp;
		cp.startTime = globalClocks[coreIndex].stop();
		cp.logicalCoreIndex = coreIndex;
		cp.frameCounter = frameCounter;

		checkpoints.emplace_back(cp);
	}

	void CpuProfiler::finishCheckpoint()
	{
		const uint32 coreIndex = CPU::getCurrentLogicalCoreIndex();
		checkpoints[checkpoints.size() - 1].endTime = globalClocks[coreIndex].stop();

		// #todo-cpu: Print this to somewhere
		if (checkpoints.size() > PURGE_BETWEEN_CHECKPOINTS) {
			purgeEverything();
		}
	}

	uint32 CpuProfiler::beginItem(uint32 threadId, const char* counterName) {
#if 0	// #todo-cpu: Assert or anonymous profile?
		CHECKF(profiles.find(threadId) != profiles.end(), "No profile exists for current thread");
#else
		if (profiles.find(threadId) == profiles.end()) {
			std::stringstream ss;
			ss << "Thread " << threadId;
			profiles.insert(std::pair<uint32, ProfilePerThread>(threadId, ProfilePerThread(threadId, ss.str())));
		}
#endif
		ProfilePerThread& currentProfile = profiles[threadId];
		ProfileItem item(counterName, currentProfile.currentTab, getGlobalClockTime());

		uint32 itemHandle = (uint32)profiles[threadId].items.size();
		currentProfile.items.emplace_back(item);
		currentProfile.currentTab += 1;
		return itemHandle;
	}

	void CpuProfiler::finishItem(uint32 itemHandle, uint32 threadId) {
		ProfilePerThread& currentProfile = profiles[threadId];
		CHECK(currentProfile.currentTab > 0);

		ProfileItem& item = currentProfile.items[itemHandle];
		item.endTime = getGlobalClockTime();
		item.elapsedMS = (item.endTime - item.startTime) * 1000.0f;

		currentProfile.currentTab -= 1;
	}

	void CpuProfiler::purgeEverything() {
		LOG(LogDebug, "[TEMP] Purge cpu profiles");

		purge_milestone.fetch_add(1);

		for (auto it = profiles.begin(); it != profiles.end(); ++it) {
			if (it->second.items.size() >= PURGE_ITEMS_AFTER) {
				it->second.clearItems();
			}
		}
		checkpoints.clear();
	}

	float CpuProfiler::getGlobalClockTime() {
		const uint32 coreIndex = CPU::getCurrentLogicalCoreIndex();
		return globalClocks[coreIndex].stop();
	}

}
