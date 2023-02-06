#include "pathos/core_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/input/input_manager.h"
using namespace pathos;

const char* WINDOW_TITLE = "Model Viewer";
const int32 WINDOW_WIDTH = 1920;
const int32 WINDOW_HEIGHT = 1080;
const bool  WINDOW_FULLSCREEN = false;

int main(int argc, char** argv) {
	EngineConfig engineConfig;
	engineConfig.windowWidth  = WINDOW_WIDTH;
	engineConfig.windowHeight = WINDOW_HEIGHT;
	engineConfig.fullscreen   = WINDOW_FULLSCREEN;
	engineConfig.title        = WINDOW_TITLE;
	Engine::init(argc, argv, engineConfig);

	gEngine->start();

	return 0;
}
