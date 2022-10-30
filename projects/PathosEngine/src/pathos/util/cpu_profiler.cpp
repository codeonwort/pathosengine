#include "cpu_profiler.h"
#include "log.h"
#include "pathos/engine.h"
#include "pathos/console.h"

#include "badger/assertion/assertion.h"
#include "badger/thread/cpu.h"

#include <fstream>
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

		// #todo-cpu: Temp logic for periodic purge
		purge_milestone = CpuProfiler::getInstance().purge_milestone.load();

		itemHandle = CpuProfiler::getInstance().beginItem(threadId, name);
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

		gEngine->registerExec("profile_cpu", [](const std::string& command) {
			int32 numFrames;
			bool bText = false;

			char fmt[16] = "json";
			int ret = sscanf_s(command.c_str(), "profile_cpu %d %s", &numFrames, fmt, (unsigned)_countof(fmt));
			if (ret <= 0) {
				numFrames = 5;
			} else if (ret == 2) {
				bText = (0 == strcmp(fmt, "text")) || (0 == strcmp(fmt, "txt"));
			}
			CpuProfiler::getInstance().dumpCPUProfile(numFrames, !bText);
		});
	}

	void CpuProfiler::registerCurrentThread(const char* inDebugName) {
		const uint32 threadId = CPU::getCurrentThreadId();
		registerThread(threadId, inDebugName);
	}
	void CpuProfiler::registerThread(uint32 threadId, const char* inDebugName) {
		std::lock_guard<std::mutex> profileLock(profilesMutex);

		CHECKF(profiles.find(threadId) == profiles.end(), "Current thread is already registered");
		profiles.insert(std::pair<uint32, ProfilePerThread>(threadId, ProfilePerThread(threadId, inDebugName)));
	}

	void CpuProfiler::beginCheckpoint(uint32 frameCounter) {
		std::lock_guard<std::mutex> cpLock(checkpointMutex);
		ProfileCheckpoint cp;
		cp.startTime = getGlobalClockTime();
		cp.frameCounter = frameCounter;
		checkpoints.emplace_back(cp);
	}

	void CpuProfiler::finishCheckpoint() {
		std::lock_guard<std::mutex> cpLock(checkpointMutex);
		checkpoints[checkpoints.size() - 1].endTime = getGlobalClockTime();
		if (checkpoints.size() > PURGE_BETWEEN_CHECKPOINTS) {
			purgeEverything();
		}
	}

	uint32 CpuProfiler::beginItem(uint32 threadId, const char* counterName) {
		std::lock_guard<std::mutex> profileLock(profilesMutex);

#if 0	// Create anonymous profile rather than assert
		CHECKF(profiles.find(threadId) != profiles.end(), "No profile exists for current thread");
#else
		if (profiles.find(threadId) == profiles.end()) {
			std::stringstream ss;
			ss << "Thread " << threadId;
			profiles.insert(std::pair<uint32, ProfilePerThread>(threadId, ProfilePerThread(threadId, ss.str())));
			LOG(LogDebug, "[%s] Anonymous thread has been detected: %s", __FUNCTION__, ss.str());
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
		std::lock_guard<std::mutex> profileLock(profilesMutex);

		ProfilePerThread& currentProfile = profiles[threadId];
		// #todo-fatal: profilesMutex won't prevent ~ScopedCpuCounter()
		// from calling finishItem(), but at least access to profiles is safe.
		// If currentTab is zero, that counter is already invalidated.
		if (currentProfile.currentTab == 0) {
			return;
		}
		//CHECK(currentProfile.currentTab > 0);

		ProfileItem& item = currentProfile.items[itemHandle];
		item.endTime = getGlobalClockTime();
		item.elapsedMS = (item.endTime - item.startTime) * 1000.0f;

		currentProfile.currentTab -= 1;
	}

	void CpuProfiler::getLastFrameSnapshot(uint32 threadID, std::vector<ProfileItem>& outSnapshot) {
		std::lock_guard<std::mutex> profileLock(profilesMutex);

		outSnapshot.clear();
		if (profiles.find(threadID) == profiles.end()) {
			return;
		}
		ProfilePerThread& profile = profiles[threadID];
		const int32 numItems = (int32)profile.items.size();
		int32 rootIx = -1;
		for (int32 i = numItems - 1; i >= 0; --i) {
			if (profile.items[i].tab == 0) {
				rootIx = i;
				break;
			}
		}
		if (rootIx != -1) {
			outSnapshot.reserve(profile.items.size() - rootIx);
			for (int32 i = rootIx; i < numItems; ++i) {
				outSnapshot.push_back(profile.items[i]);
			}
		}
	}

	void CpuProfiler::purgeEverything() {
		std::lock_guard<std::mutex> profileLock(profilesMutex);

		purge_milestone.fetch_add(1);
		// #todo-fatal: Ok, just atomic add won't save this from crash.
		// 1. fetch_add() is executed.
		// 2. New cpu counter is started.
		// 3. Profiles are cleared.
		// 4. New cpu counter is finished -> CRASH
		// Inefficient but also use profilesMutex for now.
		// Anyway all the problem comes from periodic temp purge.
		// Or... itemHandle is just an array element index,
		// maybe I'll need a way to check if itemHandle is invalidated.
		for (auto it = profiles.begin(); it != profiles.end(); ++it) {
			if (it->second.items.size() >= PURGE_ITEMS_AFTER) {
				it->second.clearItems();
			}
		}
		checkpoints.clear();
	}

	void CpuProfiler::dumpCPUProfile(int32 numFramesToDump, bool bChromeTracingFormat) {
		if (checkpoints.size() == 0 || (checkpoints.size() == 1 && checkpoints[0].endTime < 0.0f)) {
			LOG(LogInfo, "No CPU profile data exists");
			return;
		}

		std::lock_guard<std::mutex> cpLock(checkpointMutex);
		std::lock_guard<std::mutex> profileLock(profilesMutex);

		std::string filepath = pathos::getSolutionDir();
		filepath += "log/";
		pathos::createDirectory(filepath.c_str());

		time_t now = ::time(0);
		tm localTm;
		errno_t timeErr = ::localtime_s(&localTm, &now);
		CHECKF(timeErr == 0, "Failed to get current time");
		char timeBuffer[128];
		if (bChromeTracingFormat) {
			::strftime(timeBuffer, sizeof(timeBuffer), "CPUProfile-%Y-%m-%d-%H-%M-%S.json", &localTm);
		} else {
			::strftime(timeBuffer, sizeof(timeBuffer), "CPUProfile-%Y-%m-%d-%H-%M-%S.txt", &localTm);
		}
		filepath += std::string(timeBuffer);

		std::fstream fs(filepath, std::fstream::out);

		auto tab = [&fs](uint32 n) {
			while (n-- > 0) {
				fs << "    ";
			}
		};

		if (fs.is_open()) {
			// Dump per frame checkpoint
			int32 checkpointIxEnd = (int32)checkpoints.size() - 1;
			if (checkpoints[checkpointIxEnd].endTime < 0.0f) {
				checkpointIxEnd -= 1;
			}
			int32 checkpointIxStart = numFramesToDump <= 0 ? 0 : std::max(0, checkpointIxEnd - numFramesToDump);

			// #todo-cpu: Naive double loop, but not critically slow for few frames.
			// Can be optimized from O(MN) to O(M + N) if needed.
			if (bChromeTracingFormat) {
				char eventMsg[1024];
				std::vector<std::string> events;
				const uint32 pid = 0; // I don't need pid

				float dumpStartTime = checkpoints[checkpointIxStart].startTime;
				float dumpEndTime = checkpoints[checkpointIxEnd].endTime;

				fs << "[" << '\n';
				// Metadata events
				for (auto it_p = profiles.begin(); it_p != profiles.end(); ++it_p) {
					const uint32 tid = it_p->first;
					const std::string tname = it_p->second.threadName;
					sprintf_s(eventMsg, "{\"name\":\"thread_name\", \"ph\":\"M\", \"pid\":%u, \"tid\":%u, \"args\":{\"name\":\"%s\"}}",
						pid, tid, tname.c_str());
					fs << eventMsg;
					fs << ",\n";
				}
				// Events
				for (auto it_p = profiles.begin(); it_p != profiles.end(); ++it_p) {
					const uint32 tid = it_p->first;
					const ProfilePerThread& profile = it_p->second;
					size_t numItems = profile.items.size();
					for (size_t i = 0; i < numItems; ++i) {
						const ProfileItem& item = profile.items[i];
						if (dumpStartTime <= item.startTime && item.startTime <= dumpEndTime && item.elapsedMS > 0.0f) {
							sprintf_s(eventMsg, "{\"name\":\"%s\", \"cat\":\"CPU\", \"ph\":\"X\", \"ts\":%u, \"dur\":%u, \"pid\":%u, \"tid\":%u}",
								item.name, (uint32)(item.startTime * 1000 * 1000), (uint32)(item.elapsedMS * 1000), pid, tid);
							events.push_back(eventMsg);
						}
					}
				}
				for (size_t i = 0; i < events.size(); ++i) {
					fs << events[i];
					if (i != events.size() - 1) {
						fs << ',';
					}
					fs << '\n';
				}
				fs << "]" << std::endl;
			} else {
				// #todo-cpu: Text dump is missing in-between counters (checkpoints are discontinuous).
				for (int32 checkpointIx = checkpointIxStart; checkpointIx <= checkpointIxEnd; ++checkpointIx) {
					const ProfileCheckpoint& cp = checkpoints[checkpointIx];
					fs << "[Frame " << cp.frameCounter << ']' << std::endl;

					for (auto it_p = profiles.begin(); it_p != profiles.end(); ++it_p) {
						const ProfilePerThread& profile = it_p->second;

						if (profile.isEmpty()) {
							continue;
						}

						size_t numItems = profile.items.size();
						std::vector<size_t> validItemIndices;
						for (size_t i = 0; i < numItems; ++i) {
							const ProfileItem& item = profile.items[i];
							if (cp.startTime <= item.startTime && item.startTime <= cp.endTime) {
								validItemIndices.push_back(i);
							}
						}

						if (validItemIndices.size() > 0) {
							fs << profile.threadName << std::endl;
							for (size_t i = 0; i < validItemIndices.size(); ++i) {
								const ProfileItem& item = profile.items[validItemIndices[i]];
								tab(item.tab + 1);
								fs << item.name << ": " << item.elapsedMS << " ms" << std::endl;
							}
							fs << std::endl;
						}
					}
				}
			}
			fs.close();
		}

		LOG(LogDebug, "Dump CPU profile to: %s", filepath.c_str());
		gConsole->addLine("Dump CPU profile to:", false);
		gConsole->addLine(filepath.c_str(), false);
	}

	float CpuProfiler::getGlobalClockTime() {
		const uint32 coreIndex = CPU::getCurrentLogicalCoreIndex();
		return globalClocks[coreIndex].stop();
	}

}
