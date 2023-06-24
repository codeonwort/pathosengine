#include "music_record.h"

#include <algorithm>

void PlayRecord::reserve(size_t numKeyEvents) {
	laneKeyEvents.reserve(numKeyEvents);
}

void PlayRecord::addLaneKeyEvent(int32 laneIndex, float time) {
	laneKeyEvents.push_back({ laneIndex, time });
}

void PlayRecord::finalizeLoad() {
	auto comparer = [](const LaneKeyEvent& A, const LaneKeyEvent& B) {
		return A.time < B.time;
	};
	std::sort(laneKeyEvents.begin(), laneKeyEvents.end(), comparer);
}

uint32 PlayRecord::getTotalLaneKeyEvents() const {
	return (uint32)laneKeyEvents.size();
}
