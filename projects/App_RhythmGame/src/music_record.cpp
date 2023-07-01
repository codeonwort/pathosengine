#include "music_record.h"

#include <algorithm>

void PlayRecord::clearRecord(size_t numReserved) {
	laneKeyEvents.clear();
	laneKeyEvents.reserve(numReserved);
}

void PlayRecord::addShortNoteEvent(int32 laneIndex, float time) {
	laneKeyEvents.push_back({ laneIndex, time, -1.0f });
}

void PlayRecord::addLongNoteEvent(int32 laneIndex, float startTime, float endTime) {
	laneKeyEvents.push_back({ laneIndex, startTime, endTime });
}

void PlayRecord::finalizeLoad() {
	auto comparer = [](const LaneKeyEvent& A, const LaneKeyEvent& B) {
		return A.pressTime < B.pressTime;
	};
	std::sort(laneKeyEvents.begin(), laneKeyEvents.end(), comparer);
}

uint32 PlayRecord::getTotalLaneKeyEvents() const {
	return (uint32)laneKeyEvents.size();
}
