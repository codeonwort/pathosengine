#include "world2.h"
#include "pathos/core_minimal.h"
using namespace std;
using namespace pathos;

constexpr int32     WINDOW_WIDTH    = 1920;
constexpr int32     WINDOW_HEIGHT   = 1080;
constexpr char*     WINDOW_TITLE    = "Test: Skeletal Animation";
constexpr float     FOVY            = 60.0f;
const     vector3   CAMERA_POSITION = vector3(0.0f, 0.0f, 300.0f);
constexpr float     CAMERA_Z_NEAR   = 1.0f;
constexpr float     CAMERA_Z_FAR    = 10000.0f;

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = WINDOW_TITLE;
	conf.rendererType = ERendererType::Deferred;
	Engine::init(argc, argv, conf);

	const float ar = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);

	World* world2 = new World2;
	world2->getCamera().lookAt(CAMERA_POSITION, CAMERA_POSITION + vector3(0.0f, 0.0f, -1.0f), vector3(0.0f, 1.0f, 0.0f));
	world2->getCamera().changeLens(PerspectiveLens(FOVY, ar, CAMERA_Z_NEAR, CAMERA_Z_FAR));

	gEngine->setWorld(world2);
	gEngine->start();

	return 0;
}
