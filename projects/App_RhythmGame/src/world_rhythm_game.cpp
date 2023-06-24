#include "world_rhythm_game.h"
#include "bass_wrapper.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

#define KEY_RECORDS_NUM_RESERVED    16384

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

	bool bSuccess = gBass->playFromFile(TEMP_MP3_PATH);

	const char* keyNames[] = {
		"lane0", "lane1", "lane2",
		"lane3", "lane4", "lane5",
	};
	ButtonBinding noteKeys[] = {
		{ InputConstants::KEYBOARD_A },
		{ InputConstants::KEYBOARD_S },
		{ InputConstants::KEYBOARD_D },
		{ InputConstants::KEYBOARD_J },
		{ InputConstants::KEYBOARD_K },
		{ InputConstants::KEYBOARD_L },
	};

	inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	for (size_t i = 0; i < _countof(keyNames); ++i) {
		inputManager->bindButtonPressed(keyNames[i], noteKeys[i], [i, this]() {
			this->onPressLaneKey((int32)i);
		});
	}

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

	initGameTime = gEngine->getWorldTime();
}

void World_RhythmGame::onDestroy() {
	BassWrapper::destroyBASS();
}

void World_RhythmGame::onTick(float deltaSeconds) {
	currentGameTime = gEngine->getWorldTime() - initGameTime;
}

void World_RhythmGame::onPressLaneKey(int32 laneIndex) {
	LOG(LogDebug, "Press lane[%d] %f seconds", laneIndex, currentGameTime);
	playRecord.addLaneKeyEvent(laneIndex, currentGameTime);
}
