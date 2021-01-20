#include "cpu_profiler.h"
#include "badger/assertion/assertion.h"
#include "badger/thread/cpu.h"

namespace pathos {

	// #todo-cpu: Cleanup everything periodically for now
	static constexpr size_t MAX_PROFILE_CHECKPOINT = 256;

	ScopedCpuCounter::ScopedCpuCounter(const char* inName)
		: name(inName)
	{
		CHECK(inName != nullptr);
		coreIndex = CPU::getCurrentLogicalCoreIndex();
		itemHandle = CpuProfiler::getInstance().beginItem(coreIndex, name);
	}

	ScopedCpuCounter::~ScopedCpuCounter() {
		CpuProfiler::getInstance().finishItem(itemHandle, coreIndex);
	}

	//////////////////////////////////////////////////////////////////////////

	pathos::CpuProfiler& CpuProfiler::getInstance() {
		static CpuProfiler instance;
		return instance;
	}

	void CpuProfiler::initialize() {
		const uint32 nCores = CPU::getTotalLogicalCoreCount();
		CHECKF(nCores != 0, "Couldn't get total core count");
		profiles.resize(nCores);
		checkpoints.resize(nCores);
		globalClocks.resize(nCores);
		for (uint32 i = 0; i < nCores; ++i) {
			globalClocks[i].start();
		}
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
		if (checkpoints.size() > MAX_PROFILE_CHECKPOINT) {
			purgeEverything();
		}
	}

	uint32 CpuProfiler::beginItem(uint32 coreIndex, const char* counterName) {
		CHECK(coreIndex < (uint32)profiles.size());

		ProfileItem item(counterName, profiles[coreIndex].currentTab, globalClocks[coreIndex].stop());

		// #todo-cpu: It seems coreIndex changes within a single function?
		uint32 itemHandle = (uint32)profiles[coreIndex].items.size();
		profiles[coreIndex].items.emplace_back(item);
		profiles[coreIndex].currentTab += 1;
		return itemHandle;
	}

	void CpuProfiler::finishItem(uint32 frameHandle, uint32 coreIndex) {
		CHECK(coreIndex < (uint32)profiles.size());
		// #todo-cpu: A single function jumps over logical cores so this CHECK fails ??
		// #todo-fatal: Must fix
		CHECK(profiles[coreIndex].currentTab > 0);

		// #todo-cpu: Hazard - can a parent item finishes before a child item?
		ProfileItem& item = profiles[coreIndex].items[frameHandle];
		item.endTime = globalClocks[coreIndex].stop();
		item.elapsedMS = (item.endTime - item.startTime) * 1000.0f;

		profiles[coreIndex].currentTab -= 1;
	}

	void CpuProfiler::purgeEverything() {
		for (size_t i = 0; i < profiles.size(); ++i) {
			profiles[i].clear();
		}
		checkpoints.clear();
	}

}
