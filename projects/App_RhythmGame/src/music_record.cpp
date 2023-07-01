#include "music_record.h"

#include "pathos/engine.h"
#include "pathos/util/log.h"
using namespace pathos;

#include <algorithm>

void PlayRecord::clearRecord(uint32 numLanes, size_t numReserved) {
	laneCount = numLanes;

	laneKeyEvents.clear();
	laneKeyEvents.resize(laneCount);

	flattenedEvents.clear();
	flattenedEvents.reserve(numReserved);

	numEventsPerLane.clear();
	numEventsPerLane.resize(laneCount, 0);
}

void PlayRecord::addShortNoteEvent(int32 laneIndex, float time) {
	if (laneIndex < 0 || laneIndex >= (int32)laneCount) {
		LOG(LogError, "%s: Invalid lane index: %d", __FUNCTION__, laneIndex);
	} else {
		flattenedEvents.push_back({ laneIndex, time, -1.0f });
		numEventsPerLane[laneIndex] += 1;
	}
}

void PlayRecord::addLongNoteEvent(int32 laneIndex, float startTime, float endTime) {
	if (laneIndex < 0 || laneIndex >= (int32)laneCount) {
		LOG(LogError, "%s: Invalid lane index: %d", __FUNCTION__, laneIndex);
	} else {
		flattenedEvents.push_back({ laneIndex, startTime, endTime });
		numEventsPerLane[laneIndex] += 1;
	}
}

void PlayRecord::finalizeLoad() {
	for (uint32 laneIx = 0; laneIx < laneCount; ++laneIx) {
		laneKeyEvents[laneIx].reserve(numEventsPerLane[laneIx]);
	}
	for (size_t evtIx = 0; evtIx < flattenedEvents.size(); ++evtIx) {
		const LaneKeyEvent& evt = flattenedEvents[evtIx];
		laneKeyEvents[evt.laneIndex].push_back(evt);
	}

	auto comparer = [](const LaneKeyEvent& A, const LaneKeyEvent& B) {
		return A.pressTime < B.pressTime;
	};
	for (uint32 laneIx = 0; laneIx < laneCount; ++laneIx) {
		std::sort(laneKeyEvents[laneIx].begin(), laneKeyEvents[laneIx].end(), comparer);
	}
}
