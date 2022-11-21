#include "world2.h"
#include "pathos/core_minimal.h"
using namespace std;
using namespace pathos;

constexpr int32     WINDOW_WIDTH    = 1920;
constexpr int32     WINDOW_HEIGHT   = 1080;
constexpr char*     WINDOW_TITLE    = "Test: Skeletal Animation";

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = WINDOW_TITLE;
	Engine::init(argc, argv, conf);

	World* world2 = new World2;

	gEngine->setWorld(world2);
	gEngine->start();

	return 0;
}
