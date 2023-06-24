#pragma once

#include "pathos/scene/world.h"
#include "pathos/util/log.h"
using namespace pathos;

struct LaneKeyEvent {
	int32 laneIndex;
	float time;      // Elapsed seconds when key pressed
};

struct PlayRecord {
	void reserve(size_t numKeyEvents) {
		laneKeyEvents.reserve(numKeyEvents);
	}

	void addLaneKeyEvent(int32 laneIndex, float time) {
		laneKeyEvents.push_back({ laneIndex, time });
	}

	uint32 getTotalLaneKeyEvents() const {
		return (uint32)laneKeyEvents.size();
	}

	std::vector<LaneKeyEvent> laneKeyEvents;
};

class World_RhythmGame : public World {

public:
	void onPressLaneKey(int32 laneIndex);
	const PlayRecord& getPlayRecord() const { return playRecord; }

protected:
	virtual void onInitialize() override;
	virtual void onDestroy() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void initializeStage();

private:
	InputManager* inputManager = nullptr;
	float initGameTime = 0.0f;
	float currentGameTime = 0.0f;

	PlayRecord playRecord;
	GlobalFileLogger playRecordFileWriter;
};
