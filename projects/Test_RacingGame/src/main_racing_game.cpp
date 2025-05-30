#include "world/world_racing_title.h"
#include "world/world_racing_game.h"

#include "pathos/core_minimal.h"
using namespace pathos;

const char* WINDOW_TITLE = "Racing Game";
const int32 WINDOW_WIDTH = 1920;
const int32 WINDOW_HEIGHT = 1080;
const bool WINDOW_FULLSCREEN = false;

int main(int argc, char** argv) {
	EngineConfig engineConfig;
	engineConfig.windowWidth  = WINDOW_WIDTH;
	engineConfig.windowHeight = WINDOW_HEIGHT;
	engineConfig.fullscreen   = WINDOW_FULLSCREEN;
	engineConfig.title        = WINDOW_TITLE;
	Engine::init(argc, argv, engineConfig);

	World* startWorld = new World_RacingTitle;
	
	gEngine->setWorld(startWorld);
	gEngine->start();

	return 0;
}
