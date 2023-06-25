#include "world_rhythm_game.h"
#include "bass_wrapper.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/input/input_manager.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/overlay/label.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

// #todo-rhythm: image widget test
#include "pathos/loader/imageloader.h"

#include "badger/math/minmax.h"
#include <sstream>

#define KEY_RECORDS_NUM_RESERVED    16384

#define LANE_COUNT                  _countof(gLaneDesc)
#define LANE_X0                     120
#define LANE_Y0                     100
#define LANE_SPACE_X                10
#define LANE_WIDTH                  90
#define LANE_HEIGHT                 800
#define LANE_LABEL_OFFSET_X         (LANE_WIDTH / 2 - 10)
#define LANE_LABEL_OFFSET_Y         20

#define CROSSLINE_HEIGHT            10

#define JUDGE_DISPLAY_PERIOD        0.5f
#define JUDGE_TYPE_PERFECT          0
#define JUDGE_TYPE_GOOD             1
#define JUDGE_TYPE_MISS             2
#define JUDGE_COLOR                 vector3(1.0f, 1.0f, 0.1f)
#define JUDGE_COLOR_FADE            vector3(0.1f, 0.1f, 0.1f)

#define SCOREBOARD_OFFSET_X         80
#define SCORE_LABEL_Y0              100
#define SCORE_LABEL_SPACE_Y         60

// Time of seconds between appearing at the top and reaching at the bottom of lane.
#define KEY_DROP_PERIOD             1.0f
#define CATCH_RATIO_PERFECT         0.05f
#define CATCH_RATIO_GOOD            0.1f

// Input is determined as short note
// if the time diff between key press and release is less than this value.
#define SHORT_NOTE_PERIOD           0.2f

#define NOTE_WIDTH                  LANE_WIDTH
#define NOTE_HEIGHT                 40
#define NOTE_OBJECT_POOL_SIZE       100
#define NOTE_COLOR_BLUE             0
#define NOTE_COLOR_YELLOW           1

// #todo-rhythm: Temp files
#define TEMP_RECORD_LOAD_PATH       "rhythm_game_record.txt"
#define TEMP_RECORD_SAVE_PATH       "rhythm_game_record_saved.txt"
#define TEMP_MP3_PATH               "rhythm_game/testmusic.mp3"
#define TEMP_BACKGROUND_IMAGE       "rhythm_game/background.jpg"
#define TEMP_BLUE_NOTE_IMAGE        "rhythm_game/note_blue.png"
#define TEMP_YELLOW_NOTE_IMAGE      "rhythm_game/note_yellow.png"
#define TEMP_VOLUME                 0.5f

struct LaneDesc {
	std::wstring displayLabel;
	std::string inputEventName;
	ButtonBinding inputBinding;
	int32 noteColor;
};

static LaneDesc gLaneDesc[] = {
	{ L"A", "lane0", ButtonBinding({InputConstants::KEYBOARD_A }), NOTE_COLOR_BLUE   },
	{ L"S", "lane1", ButtonBinding({InputConstants::KEYBOARD_S }), NOTE_COLOR_YELLOW },
	{ L"D", "lane2", ButtonBinding({InputConstants::KEYBOARD_D }), NOTE_COLOR_BLUE   },
	{ L"J", "lane3", ButtonBinding({InputConstants::KEYBOARD_J }), NOTE_COLOR_BLUE   },
	{ L"K", "lane4", ButtonBinding({InputConstants::KEYBOARD_K }), NOTE_COLOR_YELLOW },
	{ L"L", "lane5", ButtonBinding({InputConstants::KEYBOARD_L }), NOTE_COLOR_BLUE   },
};

float getLaneX(int32 laneIndex) {
	return (float)LANE_X0 + laneIndex * (LANE_SPACE_X + LANE_WIDTH);
}
float getShortNoteY(float ratio) {
	return (LANE_Y0 - NOTE_HEIGHT / 2) + ratio * LANE_HEIGHT;
}
void getLongNoteTransform(
	float currT, float pressT, float releaseT,
	float* y, float* height)
{
	float kTop = 1.0f - badger::clamp(0.0f, (releaseT - currT) / KEY_DROP_PERIOD, 1.0f);
	float kBottom = 1.0f - badger::clamp(0.0f, (pressT - currT) / KEY_DROP_PERIOD, 1.0f);
	*y = LANE_Y0 + kTop * LANE_HEIGHT;
	*height = (badger::max)(1.0f, (kBottom - kTop) * LANE_HEIGHT);
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
	float pressTime = -1.0f;
	float releaseTime = -1.0f;

	std::string line;
	while (std::getline(archive, line)) {
		if (line.size() == 0) {
			continue;
		}
		std::stringstream ss(line);
		ss >> laneIndex >> pressTime >> releaseTime;
		if (releaseTime > 0.0f) {
			outRecord.addLongNoteEvent(laneIndex, pressTime, releaseTime);
		} else {
			outRecord.addShortNoteEvent(laneIndex, pressTime);
		}
	}

	outRecord.finalizeLoad();
}

void saveMusicRecord(GlobalFileLogger& fileWriter, const PlayRecord& playRecord, bool binaryFormat) {
	// #todo-rhythm: Support binary format
	CHECK(binaryFormat == false);

	char buf[1024];
	for (const LaneKeyEvent& evt : playRecord.laneKeyEvents) {
		sprintf_s(buf, "%d %f %f", evt.laneIndex, evt.pressTime, evt.releaseTime);
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
			if (!gConsole->isVisible()) {
				this->onPressLaneKey((int32)i);
			}
		});
		inputManager->bindButtonReleased(evtName, btnBinding, [i, this]() {
			if (!gConsole->isVisible()) {
				this->onReleaseLaneKey((int32)i);
			}
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
	laneKeyPressTimes.resize(LANE_COUNT, -1.0f);
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

	// #todo-rhythm: Background image test
	auto imageBlob = pathos::loadImage(TEMP_BACKGROUND_IMAGE);
	pathos::Brush* backgroundBrush = nullptr;
	if (imageBlob != nullptr) {
		GLuint imageTexture = pathos::createTextureFromBitmap(imageBlob, false, false, "overlay_image_test", true);
		backgroundBrush = new pathos::ImageBrush(imageTexture);
	} else {
		backgroundBrush = new pathos::SolidColorBrush(0.1f, 0.1f, 0.2f);
	}
	pathos::Rectangle* background = new pathos::Rectangle(
		(float)gEngine->getConfig().windowWidth,
		(float)gEngine->getConfig().windowHeight);
	background->setBrush(backgroundBrush);
	root->addChild(background);

	auto laneBrush = new pathos::SolidColorBrush(0.1f, 0.1f, 0.1f);

	laneNoteColumns.resize(LANE_COUNT);
	noteBrushes.reserve(LANE_COUNT);

	auto crosslineBrush = new pathos::SolidColorBrush(0.1f, 0.8f, 0.1f);
	pathos::Rectangle* crossline = new pathos::Rectangle((LANE_WIDTH + LANE_SPACE_X) * LANE_COUNT - LANE_SPACE_X, CROSSLINE_HEIGHT);
	crossline->setX(LANE_X0);
	crossline->setY(LANE_Y0 + LANE_HEIGHT - CROSSLINE_HEIGHT / 2);
	crossline->setBrush(crosslineBrush);
	root->addChild(crossline);

	// note brushes
	pathos::Brush* blueNoteBrush = nullptr;
	pathos::Brush* yellowNoteBrush = nullptr;
	{
		auto blueBlob = pathos::loadImage(TEMP_BLUE_NOTE_IMAGE);
		if (blueBlob != nullptr) {
			GLuint blueNoteTexture = pathos::createTextureFromBitmap(blueBlob, false, false, "blue_note", true);
			blueNoteBrush = new pathos::ImageBrush(blueNoteTexture);
		} else {
			blueNoteBrush = new pathos::SolidColorBrush(0.8f, 0.8f, 1.0f);
		}
		auto yellowBlob = pathos::loadImage(TEMP_YELLOW_NOTE_IMAGE);
		if (yellowBlob != nullptr) {
			GLuint yellowNoteTexture = pathos::createTextureFromBitmap(yellowBlob, false, false, "yellow_note", true);
			yellowNoteBrush = new pathos::ImageBrush(yellowNoteTexture);
		} else {
			yellowNoteBrush = new pathos::SolidColorBrush(1.0f, 1.0f, 0.2f);
		}
	}
	
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
		laneLabel->setFont("defaultLarge");
		root->addChild(laneLabel);

		auto noteBrush = (gLaneDesc[laneIndex].noteColor == NOTE_COLOR_BLUE)
			? blueNoteBrush
			: yellowNoteBrush;
		noteBrushes.push_back(noteBrush);
	}

	noteObjectPool.reserve(NOTE_OBJECT_POOL_SIZE);
	for (size_t i = 0; i < NOTE_OBJECT_POOL_SIZE; ++i) {
		noteObjectPool.push_back(new LaneNote());
	}

	judgeLabel = new pathos::Label(L"PERFECT");
	judgeLabel->setX(LANE_X0 - 90.0f + 0.5f * LANE_COUNT * (LANE_WIDTH + LANE_SPACE_X));
	judgeLabel->setY(0.5f * (LANE_Y0 + LANE_HEIGHT));
	judgeLabel->setColor(JUDGE_COLOR);
	judgeLabel->setFont("defaultLarge");
	root->addChild(judgeLabel);

	perfectLabel = new pathos::Label(L"PERFECT : 0");
	goodLabel    = new pathos::Label(L"GOOD    : 0");
	missLabel    = new pathos::Label(L"MISS    : 0");
	perfectLabel->setX(getLaneX(LANE_COUNT + 1) + SCOREBOARD_OFFSET_X);
	goodLabel->setX(perfectLabel->getX());
	missLabel->setX(perfectLabel->getX());
	perfectLabel->setY(SCORE_LABEL_Y0);
	goodLabel->setY(SCORE_LABEL_Y0 + 1 * SCORE_LABEL_SPACE_Y);
	missLabel->setY(SCORE_LABEL_Y0 + 2 * SCORE_LABEL_SPACE_Y);
	perfectLabel->setFont("defaultLarge");
	goodLabel->setFont("defaultLarge");
	missLabel->setFont("defaultLarge");
	root->addChild(perfectLabel);
	root->addChild(goodLabel);
	root->addChild(missLabel);
}

void World_RhythmGame::startMusic() {
	if (initGameTime < 0.0f) {
		std::string mp3Path = ResourceFinder::get().find(TEMP_MP3_PATH);
		if (mp3Path.size() > 0) {
			bool bSuccess = gBass->playFromFile(mp3Path.c_str(), TEMP_VOLUME);

			scoreboardData.clearScore();

			lastSearchedEventIndex = 0;
			initGameTime = gEngine->getWorldTime();
		} else {
			LOG(LogError, "Failed to open mp3 file: %s", TEMP_MP3_PATH);
		}
	}
}

void World_RhythmGame::updateNotes(float currT) {
	size_t totalEvent = loadedRecord.laneKeyEvents.size();
	for (size_t eventIndex = lastSearchedEventIndex; eventIndex < totalEvent; ++eventIndex) {
		const LaneKeyEvent& evt = loadedRecord.laneKeyEvents[eventIndex];
		// Spawn new notes.
		if (evt.pressTime - KEY_DROP_PERIOD <= currT && currT <= evt.pressTime) {
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
		if (currT < evt.pressTime - KEY_DROP_PERIOD) {
			lastSearchedEventIndex = (int32)eventIndex;
			break;
		}
	}

	// Update note positions.
	for (auto& column : laneNoteColumns) {
		for (size_t i = 0; i < column.size(); ++i) {
			LaneNote* note = column[i];

			const LaneKeyEvent& evt = loadedRecord.laneKeyEvents[note->getEventIndex()];
			bool isShortNote = evt.isShortNote();

			float pressDistance = (evt.pressTime - currT) / KEY_DROP_PERIOD;
			float releaseDistance = (evt.releaseTime - currT) / KEY_DROP_PERIOD;

			bool canCatch = pressDistance >= -CATCH_RATIO_GOOD;
			if (isShortNote) {
				note->setY(getShortNoteY(1.0f - pressDistance));
			} else {
				float noteY, noteHeight;
				getLongNoteTransform(
					currT, evt.pressTime, evt.releaseTime,
					&noteY, &noteHeight);
				note->setY(noteY);
				note->setSize(NOTE_WIDTH, noteHeight);
			}

			bool deleteShortNote = isShortNote && (canCatch == false || note->getCatched());
			bool deleteLongNote = !isShortNote && (releaseDistance < -CATCH_RATIO_GOOD);
			if (deleteShortNote || deleteLongNote) {
				if (note->getCatched() == false) {
					scoreboardData.nMiss += 1;
					setJudge(currentGameTime, JUDGE_TYPE_MISS);
				}
				noteParent->removeChild(note);
				column.erase(column.begin());
				returnNoteToPool(note);
				--i;
			}
		}
	}

	// Update visibility of judge label.
	bool bShowJudge = (currT - judgeTime <= JUDGE_DISPLAY_PERIOD);
	judgeLabel->setVisible(bShowJudge);
	// #todo-rhythm: Label scaling is bugged
	//float judgeScale = glm::mix(1.0f, 1.2f, (currT - judgeTime) / JUDGE_DISPLAY_PERIOD);
	//judgeLabel->setScaleX(judgeScale);
	//judgeLabel->setScaleY(judgeScale);
	vector3 judgeColor = glm::mix(JUDGE_COLOR, JUDGE_COLOR_FADE, (currT - judgeTime) / JUDGE_DISPLAY_PERIOD);
	judgeLabel->setColor(judgeColor);

	// Update score labels.
	wchar_t scoreText[256];
	swprintf_s(scoreText, L"PERFECT : %d", scoreboardData.nPerfect);
	perfectLabel->setText(scoreText);
	swprintf_s(scoreText, L"GOOD    : %d", scoreboardData.nGood);
	goodLabel->setText(scoreText);
	swprintf_s(scoreText, L"MISS    : %d", scoreboardData.nMiss);
	missLabel->setText(scoreText);
}

void World_RhythmGame::setJudge(float currentT, int32 judgeType) {
	judgeTime = currentT;
	if (judgeType == JUDGE_TYPE_PERFECT) {
		judgeLabel->setText(L"PERFECT");
	} else if (judgeType == JUDGE_TYPE_GOOD) {
		judgeLabel->setText(L" GOOD ");
	} else if (judgeType == JUDGE_TYPE_MISS){
		judgeLabel->setText(L" MISS ");
	}
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
	note->setSize(NOTE_WIDTH, NOTE_HEIGHT);
	return note;
}

void World_RhythmGame::returnNoteToPool(LaneNote* note) {
	noteObjectPool.push_back(note);
}

void World_RhythmGame::onPressLaneKey(int32 laneIndex) {
	// Record input
	if (laneKeyPressTimes[laneIndex] >= 0.0f) {
		LOG(LogError, "[OnPress] Previous key press was not handled (lane %d)", laneIndex);
	}
	laneKeyPressTimes[laneIndex] = currentGameTime;

	// Process current play
	for (LaneNote* note : laneNoteColumns[laneIndex]) {
		if (note->getCatched()) {
			continue;
		}

		const LaneKeyEvent& evt = loadedRecord.laneKeyEvents[note->getEventIndex()];
		bool isShortNote = evt.isShortNote();

		float ratio = std::abs((evt.pressTime - currentGameTime) / KEY_DROP_PERIOD);
		if (ratio <= CATCH_RATIO_PERFECT) {
			if (isShortNote) {
				scoreboardData.nPerfect += 1;
			} else {
				// Long note score is processed in release event.
			}
			note->setCatched(true);
			setJudge(currentGameTime, JUDGE_TYPE_PERFECT);
		} else if (ratio <= CATCH_RATIO_GOOD) {
			if (isShortNote) {
				scoreboardData.nGood += 1;
			} else {
				// Long note score is processed in release event.
			}
			note->setCatched(true);
			setJudge(currentGameTime, JUDGE_TYPE_GOOD);
		} else {
			// Assumes notes are sorted by time.
			break;
		}
	}
}

void World_RhythmGame::onReleaseLaneKey(int32 laneIndex) {
	// Record input
	float noteStartTime = laneKeyPressTimes[laneIndex];
	float noteEndTime = currentGameTime;
	float timeDiff = noteEndTime - noteStartTime;

	if (noteStartTime >= 0.0f && timeDiff > 0.0f) {
		if (timeDiff <= SHORT_NOTE_PERIOD) {
			recordToSave.addShortNoteEvent(laneIndex, noteStartTime);
		} else {
			recordToSave.addLongNoteEvent(laneIndex, noteStartTime, noteEndTime);
		}
	} else {
		LOG(LogError, "[OnRelease] Key recording is bugged (lane %d)", laneIndex);
	}

	laneKeyPressTimes[laneIndex] = -1.0f;

	// Process current play
	for (LaneNote* note : laneNoteColumns[laneIndex]) {
		if (note->getCatched()) {
			const LaneKeyEvent& evt = loadedRecord.laneKeyEvents[note->getEventIndex()];
			if (evt.isShortNote()) {
				continue;
			}

			float ratio = std::abs((evt.releaseTime - currentGameTime) / KEY_DROP_PERIOD);
			if (ratio <= CATCH_RATIO_PERFECT) {
				scoreboardData.nPerfect += 1;
				setJudge(currentGameTime, JUDGE_TYPE_PERFECT);
			} else if (ratio <= CATCH_RATIO_GOOD) {
				scoreboardData.nGood += 1;
				setJudge(currentGameTime, JUDGE_TYPE_GOOD);
			} else {
				note->setCatched(false);
				// Assumes notes are sorted by time.
				break;
			}
		}
	}
}
