#pragma once

#include "badger/types/int_types.h"

#include <vector>

struct LaneKeyEvent {
	int32 laneIndex;
	float pressTime;   // Elapsed seconds since game start when key pressed
	float releaseTime; // Negative if short note

	inline bool isShortNote() const { return releaseTime < 0.0f; }
};

struct PlayRecord {
	void clearRecord(uint32 numLanes, size_t numReserved);

	void addShortNoteEvent(int32 laneIndex, float time);
	void addLongNoteEvent(int32 laneIndex, float startTime, float endTime);

	// Invoke after adding all note events.
	void finalizeEvents();

	inline const std::vector<LaneKeyEvent>& getLaneEvents(uint32 laneIx) const {
		return laneKeyEvents[laneIx];
	}

	inline uint32 getTotalLaneKeyEvents() const {
		return (uint32)flattenedEvents.size();
	}

private:
	uint32 laneCount = 0;
	std::vector<std::vector<LaneKeyEvent>> laneKeyEvents;

	// Temp storage
	std::vector<LaneKeyEvent> flattenedEvents;
	std::vector<uint32> numEventsPerLane;
};

struct ScoreboardData {
	void clearScore() {
		nPerfect = 0;
		nGood = 0;
		nMiss = 0;
	}

	int32 nPerfect = 0;
	int32 nGood = 0;
	int32 nMiss = 0;
};
