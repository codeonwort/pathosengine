#pragma once

#include "badger/types/int_types.h"

#include <vector>

struct LaneKeyEvent {
	int32 laneIndex;
	float time;      // Elapsed seconds since game start when key pressed
};

struct PlayRecord {
	void reserve(size_t numKeyEvents);

	void addLaneKeyEvent(int32 laneIndex, float time);
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
