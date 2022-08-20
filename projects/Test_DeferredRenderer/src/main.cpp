#include "world1.h"
#include "world_rc1.h"
#include "pathos/core_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/input/input_manager.h"
using namespace pathos;

const char* WINDOW_TITLE         = "Test: Rendering Challenge 1";
const int32 WINDOW_WIDTH         = 1920;
const int32 WINDOW_HEIGHT        = 1080;
const bool  WINDOW_FULLSCREEN    = false;
const float FOVY                 = 60.0f;
const float CAMERA_Z_NEAR        = 1.0f;
const float CAMERA_Z_FAR         = 5000.0f;

void changeWorld() {
	static bool enterRC = true;

	enterRC = !enterRC;
	World* newWorld = nullptr;
	if (enterRC) {
		newWorld = new World_RC1;
		gEngine->getMainWindow()->setTitle("Test: Rendering Challenge 1");
	} else {
		newWorld = new World1;
		gEngine->getMainWindow()->setTitle("Test: Deferred Rendering");
	}

	newWorld->getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
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

	const float aspect_ratio = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);

	World* world1 = new World_RC1;
	world1->getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
	world1->getCamera().changeLens(PerspectiveLens(FOVY, aspect_ratio, CAMERA_Z_NEAR, CAMERA_Z_FAR));

	gEngine->setWorld(world1);

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
