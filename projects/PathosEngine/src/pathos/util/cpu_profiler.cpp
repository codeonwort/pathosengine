#include "cpu_profiler.h"
#include "badger/assertion/assertion.h"
#include "badger/thread/cpu.h"

namespace pathos {

	ScopedCpuCounter::ScopedCpuCounter(const char* inName)
		: name(inName)
	{
		CHECK(inName != nullptr);
		coreIndex = CPU::getCurrentCoreIndex();
		frameHandle = CpuProfiler::getInstance().pushCounter(coreIndex, name);
		start();
	}

	ScopedCpuCounter::~ScopedCpuCounter() {
		float elapsedSeconds = stop();
		CpuProfiler::getInstance().popCounter(frameHandle, coreIndex, elapsedSeconds * 1000.0f);
	}

	//////////////////////////////////////////////////////////////////////////

	pathos::CpuProfiler& CpuProfiler::getInstance() {
		static CpuProfiler instance;
		return instance;
	}

	void CpuProfiler::clearProfile() {
		uint32 nCores = CPU::getTotalCoreCount();
		profiles.resize(nCores);
		for (uint32 i = 0; i < nCores; ++i) {
			profiles[i].clear();
		}
	}

	void CpuProfiler::collectProfile() {
		// #todo-cpu: Print this to somewhere
		uint32 numProfiles = (uint32)profiles.size();
		for (uint32 i = 0; i < numProfiles; ++i) {
			if (profiles[i].isEmpty()) {
				continue;
			}
			int32 z = 0;
		}
	}

	uint32 CpuProfiler::pushCounter(uint32 coreIndex, const char* counterName) {
		CHECK(coreIndex < (uint32)profiles.size());
		uint32 frameHandle = (uint32)profiles[coreIndex].frames.size();
		profiles[coreIndex].frames.push_back(ProfileFrame(counterName, profiles[coreIndex].tab));
		profiles[coreIndex].tab += 1;
		return frameHandle;
	}

	void CpuProfiler::popCounter(uint32 frameHandle, uint32 coreIndex, float elapsedMS) {
		CHECK(coreIndex < (uint32)profiles.size());
		CHECK(profiles[coreIndex].tab > 0);
		profiles[coreIndex].frames[frameHandle].elapsedMS = elapsedMS;
		profiles[coreIndex].tab -= 1;

	}

}
