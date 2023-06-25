#pragma once

#include "music_record.h"

#include "pathos/scene/world.h"
#include "pathos/util/log.h"
using namespace pathos;

#include <vector>

namespace pathos {
	class Brush;
	class Label;
	class DisplayObject2D;
}
class LaneNote;

class World_RhythmGame : public World {

public:
	void onPressLaneKey(int32 laneIndex);
	void onReleaseLaneKey(int32 laneIndex);

protected:
	virtual void onInitialize() override;
	virtual void onDestroy() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void initializeStage();
	void startMusic();
	void updateNotes(float currentT);
	void setJudge(float currentT, int32 judgeType);

private:
	InputManager* inputManager = nullptr;
	float initGameTime = -1.0f;
	float currentGameTime = 0.0f;

	ScoreboardData scoreboardData;

	PlayRecord loadedRecord;
	int32 lastSearchedEventIndex = 0;

	PlayRecord recordToSave;
	GlobalFileLogger playRecordFileWriter;
	std::vector<float> laneKeyPressTimes;

	std::vector<std::vector<LaneNote*>> laneNoteColumns;
	std::vector<pathos::Brush*> noteBrushes;

private:
	LaneNote* allocNoteFromPool(int32 eventIndex, pathos::Brush* brush);
	void returnNoteToPool(LaneNote* note);
	std::vector<LaneNote*> noteObjectPool;
	DisplayObject2D* noteParent;

	Label* perfectLabel = nullptr;
	Label* goodLabel = nullptr;
	Label* missLabel = nullptr;

private:
	Label* judgeLabel = nullptr;
	float judgeTime = -1.0f;
};
