#include "world_rhythm_game.h"
#include "bass_wrapper.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/input/input_manager.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/util/log.h"
#include "pathos/overlay/label.h"

#include <sstream>

struct LaneDesc {
	std::wstring displayLabel;
	std::string inputEventName;
	ButtonBinding inputBinding;
	vector3 noteColor;
};

static LaneDesc gLaneDesc[] = {
	{ L"A", "lane0", ButtonBinding({InputConstants::KEYBOARD_A }), vector3(1.0f, 1.0f, 1.0f) },
	{ L"S", "lane1", ButtonBinding({InputConstants::KEYBOARD_S }), vector3(1.0f, 1.0f, 0.2f) },
	{ L"D", "lane2", ButtonBinding({InputConstants::KEYBOARD_D }), vector3(1.0f, 1.0f, 1.0f) },
	{ L"J", "lane3", ButtonBinding({InputConstants::KEYBOARD_J }), vector3(1.0f, 1.0f, 1.0f) },
	{ L"K", "lane4", ButtonBinding({InputConstants::KEYBOARD_K }), vector3(1.0f, 1.0f, 0.2f) },
	{ L"L", "lane5", ButtonBinding({InputConstants::KEYBOARD_L }), vector3(1.0f, 1.0f, 1.0f) },
};

#define KEY_RECORDS_NUM_RESERVED    16384

#define LANE_COUNT                  _countof(gLaneDesc)
#define LANE_X0                     30
#define LANE_Y0                     100
#define LANE_SPACE_X                10
#define LANE_WIDTH                  90
#define LANE_HEIGHT                 800
#define LANE_LABEL_OFFSET_X         (LANE_WIDTH / 2)
#define LANE_LABEL_OFFSET_Y         20

#define SCOREBOARD_OFFSET_X         100
#define SCORE_LABEL_Y0              100
#define SCORE_LABEL_OFFSET_Y        40

// Time of seconds between appearing at the top and reaching at the bottom of lane.
#define KEY_DROP_PERIOD             1.0f
#define CATCH_RATIO_PERFECT         0.05f
#define CATCH_RATIO_GOOD            0.1f

#define NOTE_WIDTH                  LANE_WIDTH
#define NOTE_HEIGHT                 40
#define NOTE_OBJECT_POOL_SIZE       100

// #todo-rhythm: Temp files
#define TEMP_RECORD_LOAD_PATH       "rhythm_game_record.txt"
#define TEMP_RECORD_SAVE_PATH       "rhythm_game_record_saved.txt"
#define TEMP_MP3_PATH               "F:/testmusic.mp3"

float getLaneX(int32 laneIndex) {
	return (float)LANE_X0 + laneIndex * (LANE_SPACE_X + LANE_WIDTH);
}
float getNoteY(float ratio) {
	return (LANE_Y0 - NOTE_HEIGHT / 2) + ratio * LANE_HEIGHT;
}

class LaneNote : public pathos::Rectangle {
public:
	LaneNote() : Rectangle(NOTE_WIDTH, NOTE_HEIGHT) {}
	void setEventIndex(int32 inEventIndex) { eventIndex = inEventIndex; }
	int32 getEventIndex() const { return eventIndex; }
	void setCatched(bool value) { bCatched = value; }
	bool getCatched() const { return bCatched; }
private:
	int32 eventIndex = -1;
	bool bCatched = false;
};

void loadMusicRecord(std::istream& archive, PlayRecord& outRecord) {
	int32 laneIndex = -1;
	float time = -1.0f;

	std::string line;
	while (std::getline(archive, line)) {
		std::stringstream ss(line);
		ss >> laneIndex >> time;
		outRecord.addLaneKeyEvent(laneIndex, time);
	}

	outRecord.finalizeLoad();
}

void saveMusicRecord(GlobalFileLogger& fileWriter, const PlayRecord& playRecord, bool binaryFormat) {
	// #todo-rhythm: Support binary format
	CHECK(binaryFormat == false);

	char buf[1024];
	for (const LaneKeyEvent& evt : playRecord.laneKeyEvents) {
		sprintf_s(buf, "%d %f", evt.laneIndex, evt.time);
		fileWriter.write(buf);
	}

	fileWriter.flush();
}

void World_RhythmGame::onInitialize() {
	BassWrapper::initializeBASS();

	if (gConsole->isVisible()) {
		gConsole->toggle();
	}

	// Input
	inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	for (size_t i = 0; i < LANE_COUNT; ++i) {
		const char* evtName = gLaneDesc[i].inputEventName.c_str();
		ButtonBinding& btnBinding = gLaneDesc[i].inputBinding;
		inputManager->bindButtonPressed(evtName, btnBinding, [i, this]() {
			this->onPressLaneKey((int32)i);
		});
	}
	inputManager->bindButtonPressed("startGame", ButtonBinding({ InputConstants::SPACE }), [this]() {
		this->startMusic();
	});
	
	// Record (load)
	{
		std::string recordPath;
		std::wstring wExeDir;
		pathos::getExecDir(wExeDir);
		pathos::WCHAR_TO_MBCS(wExeDir, recordPath);
		recordPath += TEMP_RECORD_LOAD_PATH;
		std::fstream archive(recordPath, std::ios::in);
		if (archive.is_open()) {
			loadMusicRecord(archive, loadedRecord);
			LOG(LogDebug, "Load record (%u): %s", loadedRecord.getTotalLaneKeyEvents(), recordPath.c_str());
		} else {
			LOG(LogError, "Failed to load record: %s", recordPath.c_str());
		}
	}

	// Record (save)
	playRecordFileWriter.initialize(TEMP_RECORD_SAVE_PATH);
	recordToSave.reserve(KEY_RECORDS_NUM_RESERVED);
	gEngine->registerConsoleCommand("dump_play_record", [this](const std::string& command) {
		auto& fileWriter = this->playRecordFileWriter;
		auto& record = this->recordToSave;
		saveMusicRecord(fileWriter, record, false);

		wchar_t msg[256];
		swprintf_s(msg, L"Dumped %u events to: %S",
			record.getTotalLaneKeyEvents(),
			fileWriter.getFilepath().c_str());
		gConsole->addLine(msg, false, true);
	});

	// Graphics
	initializeStage();
}

void World_RhythmGame::onDestroy() {
	BassWrapper::destroyBASS();
}

void World_RhythmGame::onTick(float deltaSeconds) {
	currentGameTime = gEngine->getWorldTime() - initGameTime;
	if (initGameTime >= 0.0f) {
		updateNotes(currentGameTime);
	}
}

void World_RhythmGame::initializeStage() {
	DisplayObject2D* root = gEngine->getOverlayRoot();
	noteParent = root;

	auto laneBrush = new pathos::SolidColorBrush(0.1f, 0.1f, 0.1f);

	laneNoteColumns.resize(LANE_COUNT);
	noteBrushes.reserve(LANE_COUNT);
	
	for (uint32 laneIndex = 0; laneIndex < LANE_COUNT; ++laneIndex) {
		pathos::Rectangle* laneColumn = new pathos::Rectangle(LANE_WIDTH, LANE_HEIGHT);
		laneColumn->setX(getLaneX((int32)laneIndex));
		laneColumn->setY((float)LANE_Y0);
		laneColumn->setBrush(laneBrush);
		root->addChild(laneColumn);

		const wchar_t* labelText = gLaneDesc[laneIndex].displayLabel.c_str();
		pathos::Label* laneLabel = new pathos::Label(labelText);
		laneLabel->setX(laneColumn->getX() + LANE_LABEL_OFFSET_X);
		laneLabel->setY(laneColumn->getY() + LANE_HEIGHT + LANE_LABEL_OFFSET_Y);
		root->addChild(laneLabel);

		pathos::SolidColorBrush* noteBrush = new pathos::SolidColorBrush(
			gLaneDesc[laneIndex].noteColor.r,
			gLaneDesc[laneIndex].noteColor.g,
			gLaneDesc[laneIndex].noteColor.b);
		noteBrushes.push_back(noteBrush);
	}

	noteObjectPool.reserve(NOTE_OBJECT_POOL_SIZE);
	for (size_t i = 0; i < NOTE_OBJECT_POOL_SIZE; ++i) {
		noteObjectPool.push_back(new LaneNote());
	}

	perfectLabel = new pathos::Label(L"PERFECT : 0");
	goodLabel    = new pathos::Label(L"GOOD    : 0");
	missLabel    = new pathos::Label(L"MISS    : 0");
	perfectLabel->setX(getLaneX(LANE_COUNT + 1) + SCOREBOARD_OFFSET_X);
	goodLabel->setX(perfectLabel->getX());
	missLabel->setX(perfectLabel->getX());
	perfectLabel->setY(SCORE_LABEL_Y0);
	goodLabel->setY(SCORE_LABEL_Y0 + 1 * SCORE_LABEL_OFFSET_Y);
	missLabel->setY(SCORE_LABEL_Y0 + 2 * SCORE_LABEL_OFFSET_Y);
	root->addChild(perfectLabel);
	root->addChild(goodLabel);
	root->addChild(missLabel);
}

void World_RhythmGame::startMusic() {
	if (initGameTime < 0.0f) {
		bool bSuccess = gBass->playFromFile(TEMP_MP3_PATH);

		scoreboardData.clearScore();

		lastSearchedEventIndex = 0;
		initGameTime = gEngine->getWorldTime();
	}
}

void World_RhythmGame::updateNotes(float currT) {
	size_t totalEvent = loadedRecord.laneKeyEvents.size();
	for (size_t eventIndex = lastSearchedEventIndex; eventIndex < totalEvent; ++eventIndex) {
		const LaneKeyEvent& evt = loadedRecord.laneKeyEvents[eventIndex];
		// Spawn new notes.
		if (evt.time - KEY_DROP_PERIOD <= currT && currT <= evt.time) {
			std::vector<LaneNote*>& column = laneNoteColumns[evt.laneIndex];
			bool shouldSpawn = true;
			for (LaneNote* note : column) {
				if (note->getEventIndex() == eventIndex) {
					shouldSpawn = false;
					break;
				}
			}
			if (shouldSpawn) {
				LaneNote* newNote = allocNoteFromPool((int32)eventIndex, noteBrushes[evt.laneIndex]);
				newNote->setX(getLaneX(evt.laneIndex));
				column.push_back(newNote);
				noteParent->addChild(newNote);
			}
		}

		// Assumes events are sorted by time.
		if (currT < evt.time - KEY_DROP_PERIOD) {
			lastSearchedEventIndex = (int32)eventIndex;
			break;
		}
	}

	// Update note positions.
	for (auto& column : laneNoteColumns) {
		for (size_t i = 0; i < column.size(); ++i) {
			LaneNote* note = column[i];
			const LaneKeyEvent& evt = loadedRecord.laneKeyEvents[note->getEventIndex()];
			float yRatio = (evt.time - currT) / KEY_DROP_PERIOD;
			bool canCatch = yRatio >= -CATCH_RATIO_GOOD;
			if (canCatch) {
				note->setY(getNoteY(1.0f - yRatio));
			}
			if (canCatch == false || note->getCatched()) {
				if (note->getCatched() == false) {
					scoreboardData.nMiss += 1;
				}
				noteParent->removeChild(note);
				column.erase(column.begin());
				returnNoteToPool(note);
				--i;
			}
		}
	}

	// Update score labels.
	wchar_t scoreText[256];
	swprintf_s(scoreText, L"PERFECT : %d", scoreboardData.nPerfect);
	perfectLabel->setText(scoreText);
	swprintf_s(scoreText, L"GOOD    : %d", scoreboardData.nGood);
	goodLabel->setText(scoreText);
	swprintf_s(scoreText, L"MISS    : %d", scoreboardData.nMiss);
	missLabel->setText(scoreText);
}

LaneNote* World_RhythmGame::allocNoteFromPool(int32 eventIndex, pathos::Brush* brush) {
	LaneNote* note = nullptr;

	if (noteObjectPool.size() > 0) {
		note = noteObjectPool[noteObjectPool.size() - 1];
		noteObjectPool.pop_back();
	} else {
		LOG(LogWarning, "Note pool is empty; creating a new instance");
		note = new LaneNote();
	}

	note->setCatched(false);
	note->setEventIndex(eventIndex);
	note->setBrush(brush);
	return note;
}

void World_RhythmGame::returnNoteToPool(LaneNote* note) {
	noteObjectPool.push_back(note);
}

void World_RhythmGame::onPressLaneKey(int32 laneIndex) {
	recordToSave.addLaneKeyEvent(laneIndex, currentGameTime);
	for (LaneNote* note : laneNoteColumns[laneIndex]) {
		if (note->getCatched()) {
			continue;
		}
		const LaneKeyEvent& evt = loadedRecord.laneKeyEvents[note->getEventIndex()];
		float ratio = std::abs((evt.time - currentGameTime) / KEY_DROP_PERIOD);
		if (ratio <= CATCH_RATIO_PERFECT) {
			scoreboardData.nPerfect += 1;
			note->setCatched(true);
		} else if (ratio <= CATCH_RATIO_GOOD) {
			scoreboardData.nGood += 1;
			note->setCatched(true);
		} else {
			// Assumes notes are sorted by time.
			break;
		}
	}
}
