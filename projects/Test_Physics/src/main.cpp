#include "world_gjk.h"

#include "pathos/core_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/input/input_manager.h"
using namespace pathos;

const char* WINDOW_TITLE = "Demo Application";
const int32 WINDOW_WIDTH = 1920;
const int32 WINDOW_HEIGHT = 1080;
const bool  WINDOW_FULLSCREEN = false;

int main(int argc, char** argv) {
	EngineConfig config;
	config.windowWidth = WINDOW_WIDTH;
	config.windowHeight = WINDOW_HEIGHT;
	config.fullscreen = WINDOW_FULLSCREEN;
	config.title = WINDOW_TITLE;
	Engine::init(argc, argv, config);

	World* world = new World_GJK;
	gEngine->setWorld(world);
	gEngine->start();

	return 0;
}
