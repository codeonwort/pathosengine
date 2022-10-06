#include "world1.h"
#include "world_rc1.h"
#include "world_lightroom.h"
#include "world_sponza.h"

#include "pathos/core_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/input/input_manager.h"
using namespace pathos;

const char* WINDOW_TITLE         = "Demo Application";
const int32 WINDOW_WIDTH         = 1920;
const int32 WINDOW_HEIGHT        = 1080;
const bool  WINDOW_FULLSCREEN    = false;

void changeWorld() {
	static const int32 numWorlds = 3;
	static int32 worldIndex = 0;

	World* newWorld = nullptr;
	switch (worldIndex) {
	case 0:
		newWorld = new World_RC1;
		gEngine->getMainWindow()->setTitle("Rendering Challenge 1");
		break;
	case 1:
		newWorld = new World1;
		gEngine->getMainWindow()->setTitle("Some Random World");
		break;
	case 2:
		newWorld = new World_LightRoom;
		gEngine->getMainWindow()->setTitle("Light Room");
		break;
	// #todo-gltf
	//case 3:
	//	newWorld = new World_Sponza;
	//	gEngine->getMainWindow()->setTitle("Yet another Sponza");
	//	break;
	case 3:
		// More worlds here...
		break;
	default:
		CHECK_NO_ENTRY();
		break;
	}
	worldIndex = (worldIndex + 1) % numWorlds;

	gEngine->setWorld(newWorld);
}

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.fullscreen   = WINDOW_FULLSCREEN;
	conf.title        = WINDOW_TITLE;
	Engine::init(argc, argv, conf);

	changeWorld();

	// Allow switching between worlds
	{
		ButtonBinding switchWorldBinding;
		switchWorldBinding.addInput(InputConstants::KEYBOARD_P);
		gEngine->getInputSystem()->getDefaultInputManager()->bindButtonPressed("switchWorld", switchWorldBinding, changeWorld);

		gConsole->addLine(L"Press 'P' to switch between sample worlds.");
	}

	gEngine->start();

	return 0;
}
