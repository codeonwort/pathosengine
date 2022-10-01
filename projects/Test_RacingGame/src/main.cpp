#include "world_game1.h"
#include "pathos/core_minimal.h"
using namespace pathos;

const char*         WINDOW_TITLE = "Test: Racing Game";
const int32         WINDOW_WIDTH = 1920;
const int32         WINDOW_HEIGHT = 1080;
const bool          WINDOW_FULLSCREEN = false;
const float         FOVY = 60.0f;
const float         CAMERA_Z_NEAR = 1.0f;
const float         CAMERA_Z_FAR = 5000.0f;

#define WORLD_CLASS World_Game1

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.fullscreen = WINDOW_FULLSCREEN;
	conf.title = WINDOW_TITLE;
	Engine::init(argc, argv, conf);

	const float aspect_ratio = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);

	World* world1 = new WORLD_CLASS;
	world1->getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
	world1->getCamera().changeLens(PerspectiveLens(FOVY, aspect_ratio, CAMERA_Z_NEAR, CAMERA_Z_FAR));
	
	gEngine->setWorld(world1);
	gEngine->start();

	return 0;
}
