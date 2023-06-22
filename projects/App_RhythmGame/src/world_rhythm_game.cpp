#include "world_rhythm_game.h"
#include "bass_wrapper.h"

#define MP3_PATH "F:/testmusic.mp3"

void World_RhythmGame::onInitialize() {
	BassWrapper::initializeBASS();

	bool bSuccess = gBass->playFromFile(MP3_PATH);
}

void World_RhythmGame::onTick(float deltaSeconds) {
	//
}

void World_RhythmGame::onDestroy() {
	BassWrapper::destroyBASS();
}
