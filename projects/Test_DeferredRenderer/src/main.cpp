#include "world1.h"
#include "world_rc1.h"
#include "world_lightroom.h"

#include "pathos/core_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/input/input_manager.h"
using namespace pathos;

const char* WINDOW_TITLE         = "Deferred Shading Renderer";
const int32 WINDOW_WIDTH         = 1920;
const int32 WINDOW_HEIGHT        = 1080;
const bool  WINDOW_FULLSCREEN    = false;
const float FOV_Y                = 60.0f;
const float CAMERA_Z_NEAR        = 1.0f;
const float CAMERA_Z_FAR         = 5000.0f;

void changeWorld() {
	static const int32 numWorlds = 3;
	static int32 worldIndex = 2;

	World* newWorld = nullptr;
	switch (worldIndex) {
	case 0:
		newWorld = new World_RC1;
		gEngine->getMainWindow()->setTitle("Test: Rendering Challenge 1");
		break;
	case 1:
		newWorld = new World1;
		gEngine->getMainWindow()->setTitle("Test: Deferred Rendering");
		break;
	case 2:
		newWorld = new World_LightRoom;
		gEngine->getMainWindow()->setTitle("Test: Light Room");
		break;
	case 3:
		// More worlds here...
		break;
	default:
		CHECK_NO_ENTRY();
		break;
	}

	worldIndex = (worldIndex + 1) % numWorlds;

	float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT); // Will be updated in the engine loop anyway.
	newWorld->getCamera().changeLens(PerspectiveLens(FOV_Y, aspectRatio, CAMERA_Z_NEAR, CAMERA_Z_FAR));

	gEngine->setWorld(newWorld);
}

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.fullscreen   = WINDOW_FULLSCREEN;
	conf.title        = WINDOW_TITLE;
	conf.rendererType = ERendererType::Deferred;
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
