#pragma once

#include "music_record.h"
#include "music_database.h"
#include "bass_wrapper.h"

#include "pathos/scene/world.h"
#include "pathos/util/log.h"
using namespace pathos;

#include <vector>

namespace pathos {
	class Brush;
	class ImageBrush;
	class Label;
	class Rectangle;
	class DisplayObject2D;
}
class LaneNote;
class CatchEffect;
class MusicBrowserWidget;

enum class GameState {
	BrowseMusic,
	PlaySession,
};

class World_RhythmGame : public World {

public:
	void setMusicVolume(float value);
	void browseMusicList(int32 delta);
	void onPressLaneKey(int32 laneIndex);
	void onReleaseLaneKey(int32 laneIndex);

protected:
	virtual void onInitialize() override;
	virtual void onDestroy() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void initializeBrowseStage();
	void initializePlayStage();

	void startMusic();
	void updateNotes(float currentT);
	void setJudge(float currentT, int32 judgeType);

// App state
private:
	GameState gameState = GameState::BrowseMusic;
	InputManager* inputManager = nullptr;
	float initGameTime = -1.0f;
	float currentGameTime = 0.0f;

	// Load
	MusicDatabase musicDatabase;
	PlayRecord loadedRecord;

	// Save
	PlayRecord recordToSave;
	GlobalFileLogger playRecordFileWriter;
	std::vector<float> laneKeyPressTimes;

// Browse stage
private:
	MusicBrowserWidget* browserWidget = nullptr;

// Play stage
private:
	DisplayObject2D* playContainer = nullptr;
	
	BassStream* musicStream = nullptr;
	float musicVolume = 1.0f;
	bool bMusicStarted = false;

	ScoreboardData scoreboardData;
	int32 lastSearchedEventIndex = 0;

	std::vector<std::vector<LaneNote*>> laneNoteColumns;
	std::vector<pathos::Brush*> noteBrushes;
	std::vector<pathos::Rectangle*> lanePressEffects;

	std::vector<CatchEffect*> laneCatchffects;
	bool bCatchEffectAllValid = false;

	int32 autoPlaySearchStartIndex = 0;

private:
	LaneNote* allocNoteFromPool(int32 eventIndex, pathos::Brush* brush);
	void returnNoteToPool(LaneNote* note);
	std::vector<LaneNote*> noteObjectPool;
	DisplayObject2D* noteContainer;

	pathos::Rectangle* scoreboardRect = nullptr;
	Label* perfectLabel = nullptr;
	Label* goodLabel = nullptr;
	Label* missLabel = nullptr;

	Label* countdownLabel = nullptr;

	Label* judgeLabel = nullptr;
	float judgeTime = -100.0f;

	pathos::Rectangle* background = nullptr;
	pathos::Brush* backgroundFallbackBrush = nullptr;
	pathos::ImageBrush* backgroundImageBrush = nullptr;
};
