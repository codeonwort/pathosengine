#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/render/sky_ansel.h"
#include "pathos/text/textmesh.h"
using namespace pathos;

#include <thread>

#define LOAD_3D_MODEL   1
#define USE_ANSEL_SKY   1

// Rendering configurations
const int WINDOW_WIDTH          = 1920;
const int WINDOW_HEIGHT         = 1080;
const float FOV                 = 60.0f;
const float NEAR_Z              = 1.0f;
const float FAR_Z               = 10000.0f;
const glm::vec3 CAMERA_POSITION = glm::vec3(-200.0f, 150.0f, 200.0f);
const ERendererType RENDERER    = ERendererType::Deferred;

Camera* cam;
Scene scene;
	DirectionalLight *dirLight;
	PointLight *pointLight1, *pointLight2;
	Mesh *lightMarker1, *lightMarker2;
	Mesh *city, *city2;
	TextMesh *label;

OBJLoader cityLoader;
bool loaderReady = false;

void setupScene();
void setupSky();
void tick();

void loadTask() {
	cityLoader.load("models/city/The_City.obj", "models/city/");
	//cityLoader.load("lighting_challenges/artStudio.obj", "lighting_challenges/");
	//cityLoader.load("models/medieval/Medieval_City.obj", "models/medieval/");
	//cityLoader.load("models/dragon.obj", "models/");
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
	cam = new Camera(new PerspectiveLens(FOV, aspectRatio, NEAR_Z, FAR_Z));
	cam->move(CAMERA_POSITION);

	setupScene();
	setupSky();

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

void setupScene() {
	pointLight1 = new PointLight(glm::vec3(-1000, 100, 50), 1.0f * glm::vec3(1, 0, 0));
	pointLight2 = new PointLight(glm::vec3(-200, 100, 50), 1.0f * glm::vec3(1, 1, 1));
	dirLight = new DirectionalLight(glm::vec3(0, -1, -1), 1.0f * glm::vec3(1, 1, 1));

	auto lightBulb = new SphereGeometry(20.0f, 10);
	auto lightMat = new WireframeMaterial(1.0f, 1.0f, 1.0f);
	lightMarker1 = new Mesh(lightBulb, lightMat);
	lightMarker2 = new Mesh(lightBulb, lightMat);
	lightMarker1->getTransform().appendMove(pointLight1->getPosition());
	lightMarker2->getTransform().appendMove(pointLight2->getPosition());
	scene.add(lightMarker1);
	scene.add(lightMarker2);

#if LOAD_3D_MODEL
	label = new TextMesh("default");
	label->setText("Loading OBJ model. Please wait...", 0xff0000);
	label->getTransform().appendScale(20);
	label->doubleSided = true;
#endif

	scene.add(pointLight1);
	scene.add(pointLight2);
	scene.add(dirLight);
#if LOAD_3D_MODEL
	scene.add(label);
#endif
}

void setupSky() {
#if USE_ANSEL_SKY
	//auto anselImage = pathos::loadImage("ansel/dishonored.jpg");
	auto anselImage = pathos::loadImage("ansel/the_surge.jpg");
	GLuint anselTex = loadTexture(anselImage, true, true);
	scene.sky = new AnselSkyRendering(anselTex);
#else
	const char* cubeImgName[6] = {
	"cubemap1/pos_x.jpg", "cubemap1/neg_x.jpg",
	"cubemap1/pos_y.jpg", "cubemap1/neg_y.jpg",
	"cubemap1/pos_z.jpg", "cubemap1/neg_z.jpg"
	};
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);

	scene.sky = new Skybox(cubeTex);
#endif
}

void tick() {
	if (loaderReady) {
		loaderReady = false;
		city = cityLoader.craftMeshFromAllShapes();
		//city->getTransform().appendScale(100.0f);
		city->getTransform().appendMove(0.0f, 0.0f, 0.0f);

		scene.add(city);
		cityLoader.unload();
		label->visible = false;
	}

	if (gConsole->isVisible() == false) {
		float speedX = 5.0f, speedY = 5.0f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -3.0f : gEngine->isDown('e') ? 3.0f : 0.0f;
		float rotX = gEngine->isDown('z') ? -1.5f : gEngine->isDown('x') ? 1.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}
}
