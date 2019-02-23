#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/text/textmesh.h"
using namespace pathos;

#include <iostream>
#include <thread>
using namespace std;

Camera* cam;
Scene scene;
	Mesh *city, *city2;
	TextMesh *label;
	Skybox* sky;

PointLight *plight, *plight2;
DirectionalLight *dlight;

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
	// engine configuration
	EngineConfig conf;
	conf.windowWidth  = 1600;
	conf.windowHeight = 1200;
	conf.title        = "Test: Loading Wavefront OBJ Model";
	conf.rendererType = ERendererType::Deferred;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	cam = new Camera(new PerspectiveLens(45.0f, static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight), 1.0f, 1000.f));
	cam->move(glm::vec3(0, 0, 20));

	setupModel();
	setupSkybox();

	std::thread loadWorker(loadTask);

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	loadWorker.join();

	return 0;
}

void setupModel() {
	plight = new PointLight(glm::vec3(5, 30, 5), glm::vec3(1, 1, 1));
	plight2 = new PointLight(glm::vec3(-15, 30, 5), glm::vec3(0, 0, 1));
	dlight = new DirectionalLight(glm::vec3(0.1, -1, 2), glm::vec3(1, 1, 1));

	label = new TextMesh("default");
	label->setText("Loading OBJ model. Please wait...", 0xff0000);
	label->getTransform().appendScale(20);

	scene.add(plight);
	scene.add(dlight);
	scene.add(label);
}

void setupSkybox() {
	const char* cubeImgName[6] = {
		"cubemap1/pos_x.jpg", "cubemap1/neg_x.jpg",
		"cubemap1/pos_y.jpg", "cubemap1/neg_y.jpg",
		"cubemap1/pos_z.jpg", "cubemap1/neg_z.jpg"
	};
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	scene.sky = sky;
}

void tick() {
	if (loaderReady) {
		loaderReady = false;
		city = cityLoader.craftMeshFromAllShapes();
		city->getTransform().appendScale(0.1f, 0.1f, 0.1f);
		city->getTransform().appendMove(0, -10, 0);

		scene.add(city);
		cityLoader.unload();
		label->visible = false;
	}

	if (gConsole->isVisible() == false) {
		float speedX = 0.5f, speedY = 0.5f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -0.5f : gEngine->isDown('e') ? 0.5f : 0.0f;
		float rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}
}
