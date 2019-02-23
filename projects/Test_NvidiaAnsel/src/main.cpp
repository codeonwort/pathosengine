#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/render/sky_ansel.h"
#include "pathos/text/textmesh.h"
using namespace pathos;

#include <thread>

#define LOAD_3D_MODEL         1

// Rendering configurations
const int WINDOW_WIDTH          = 1920;
const int WINDOW_HEIGHT         = 1080;
const float FOV                 = 120.0f;
const glm::vec3 CAMERA_POSITION = glm::vec3(0.0f, 0.0f, 20.0f);
const ERendererType RENDERER    = ERendererType::Forward;

Camera* cam;
Scene scene;
	AnselSkyRendering* sky;
	Mesh *city, *city2;
	TextMesh *label;
	PointLight *pointLight1, *pointLight2;
	DirectionalLight *dirLight;

OBJLoader cityLoader;
bool loaderReady = false;

void setupModel();
void setupSkybox();
void tick();

void loadTask() {
	cityLoader.load("models/city/The_City.obj", "models/city/");
	loaderReady = true;
}

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = "Test: Nvidia Ansel";
	conf.rendererType = RENDERER;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	const float aspectRatio = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, aspectRatio, 1.0f, 1000.f));
	cam->move(CAMERA_POSITION);

	setupModel();
	setupSkybox();

#if LOAD_3D_MODEL
	std::thread loadWorker(loadTask);
#endif

	gEngine->setWorld(&scene, cam);
	gEngine->start();

#if LOAD_3D_MODEL
	loadWorker.join();
#endif

	return 0;
}

void setupModel() {
	pointLight1 = new PointLight(glm::vec3(5, 30, 5), glm::vec3(1, 1, 1));
	pointLight2 = new PointLight(glm::vec3(-15, 30, 5), glm::vec3(0, 0, 1));
	dirLight = new DirectionalLight(glm::vec3(0.1, -1, 2), glm::vec3(1, 1, 1));

#if LOAD_3D_MODEL
	label = new TextMesh("default");
	label->setText("Loading OBJ model. Please wait...", 0xff0000);
	label->getTransform().appendScale(20);
	label->doubleSided = true;
#endif

	scene.add(pointLight1);
	scene.add(dirLight);
#if LOAD_3D_MODEL
	scene.add(label);
#endif
}

void setupSkybox() {
	//auto anselImage = pathos::loadImage("ansel/dishonored.jpg");
	auto anselImage = pathos::loadImage("ansel/the_surge.jpg");
	GLuint anselTex = loadTexture(anselImage);
	scene.sky = new AnselSkyRendering(anselTex);
}

void tick() {
	if (loaderReady) {
		loaderReady = false;
		city = cityLoader.craftMeshFromAllShapes();
		city->getTransform().appendScale(0.1f, 0.1f, 0.1f);
		city->getTransform().appendMove(0.0f, -60.0f, 0.0f);

		scene.add(city);
		cityLoader.unload();
		label->visible = false;
	}

	if (gConsole->isVisible() == false) {
		float speedX = 0.4f, speedY = 0.4f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -1.0f : gEngine->isDown('e') ? 1.0f : 0.0f;
		float rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}
}
