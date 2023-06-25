#pragma once

#include "badger/types/int_types.h"

#include <vector>

struct LaneKeyEvent {
	int32 laneIndex;
	float pressTime;   // Elapsed seconds since game start when key pressed
	float releaseTime; // Negative if short note
};

struct PlayRecord {
	void reserve(size_t numKeyEvents);

	void addShortNoteEvent(int32 laneIndex, float time);
	void addLongNoteEvent(int32 laneIndex, float startTime, float endTime);
	void finalizeLoad();

	uint32 getTotalLaneKeyEvents() const;

public:
	std::vector<LaneKeyEvent> laneKeyEvents;
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
