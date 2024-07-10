#include "world_racing_game.h"
#include "pathos/core_minimal.h"
using namespace pathos;

const char* WINDOW_TITLE = "Test: Racing Game";
const int32 WINDOW_WIDTH = 1920;
const int32 WINDOW_HEIGHT = 1080;
const bool WINDOW_FULLSCREEN = false;

#define WORLD_CLASS World_Game1

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.fullscreen = WINDOW_FULLSCREEN;
	conf.title = WINDOW_TITLE;
	Engine::init(argc, argv, conf);

	World* world1 = new WORLD_CLASS;
	
	gEngine->setWorld(world1);
	gEngine->start();

	return 0;
}
