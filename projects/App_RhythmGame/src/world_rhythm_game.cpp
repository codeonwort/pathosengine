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

struct LaneDesc {
	std::wstring displayLabel;
	std::string inputEventName;
	ButtonBinding inputBinding;
};

static LaneDesc gLaneDesc[] = {
	{ L"A", "lane0", ButtonBinding({InputConstants::KEYBOARD_A }) },
	{ L"S", "lane1", ButtonBinding({InputConstants::KEYBOARD_S }) },
	{ L"D", "lane2", ButtonBinding({InputConstants::KEYBOARD_D }) },
	{ L"J", "lane3", ButtonBinding({InputConstants::KEYBOARD_J }) },
	{ L"K", "lane4", ButtonBinding({InputConstants::KEYBOARD_K }) },
	{ L"L", "lane5", ButtonBinding({InputConstants::KEYBOARD_L }) },
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

// #todo-rhythm: Temp files
#define TEMP_RECORD_PATH            "rhythm_game_record.txt"
#define TEMP_MP3_PATH               "F:/testmusic.mp3"

void dumpPlayRecord(GlobalFileLogger& fileWriter, const PlayRecord& playRecord, bool binaryFormat) {
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

	// Music
	bool bSuccess = gBass->playFromFile(TEMP_MP3_PATH);

	// Input
	inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	for (size_t i = 0; i < LANE_COUNT; ++i) {
		const char* evtName = gLaneDesc[i].inputEventName.c_str();
		ButtonBinding& btnBinding = gLaneDesc[i].inputBinding;
		inputManager->bindButtonPressed(evtName, btnBinding, [i, this]() {
			this->onPressLaneKey((int32)i);
		});
	}

	// Record
	playRecordFileWriter.initialize(TEMP_RECORD_PATH);
	playRecord.reserve(KEY_RECORDS_NUM_RESERVED);
	gEngine->registerConsoleCommand("dump_play_record", [this](const std::string& command) {
		auto& fileWriter = this->playRecordFileWriter;
		auto& record = this->playRecord;
		dumpPlayRecord(fileWriter, record, false);

		wchar_t msg[256];
		swprintf_s(msg, L"Dumped %u events to: %S",
			record.getTotalLaneKeyEvents(),
			fileWriter.getFilepath().c_str());
		gConsole->addLine(msg, false, true);
	});

	// Graphics
	initializeStage();

	// Time
	initGameTime = gEngine->getWorldTime();
}

void World_RhythmGame::onDestroy() {
	BassWrapper::destroyBASS();
}

void World_RhythmGame::onTick(float deltaSeconds) {
	currentGameTime = gEngine->getWorldTime() - initGameTime;
}

void World_RhythmGame::initializeStage() {
	DisplayObject2D* root = gEngine->getOverlayRoot();

	auto laneBrush = new pathos::SolidColorBrush(0.1f, 0.1f, 0.1f);
	
	for (uint32 laneIndex = 0; laneIndex < LANE_COUNT; ++laneIndex) {
		pathos::Rectangle* laneColumn = new pathos::Rectangle(LANE_WIDTH, LANE_HEIGHT);
		laneColumn->setX((float)LANE_X0 + laneIndex * (LANE_SPACE_X + LANE_WIDTH));
		laneColumn->setY((float)LANE_Y0);
		laneColumn->setBrush(laneBrush);
		root->addChild(laneColumn);

		const wchar_t* labelText = gLaneDesc[laneIndex].displayLabel.c_str();
		pathos::Label* laneLabel = new pathos::Label(labelText);
		laneLabel->setX(laneColumn->getX() + LANE_LABEL_OFFSET_X);
		laneLabel->setY(laneColumn->getY() + LANE_HEIGHT + LANE_LABEL_OFFSET_Y);
		root->addChild(laneLabel);
	}
}

void World_RhythmGame::onPressLaneKey(int32 laneIndex) {
	playRecord.addLaneKeyEvent(laneIndex, currentGameTime);
}
