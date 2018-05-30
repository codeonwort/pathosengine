#include "glm/gtx/transform.hpp"
#include "pathos/engine.h"
#include "pathos/render/render_forward.h"
#include "pathos/render/envmap.h"
#include "pathos/mesh/mesh.h"
#include "pathos/camera/camera.h"
#include "pathos/light/light.h"
#include "pathos/loader/imageloader.h"
#include "pathos/loader/objloader.h"
#include "pathos/text/textmesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/resource_finder.h"

#include <iostream>
#include <thread>

using namespace std;
using namespace pathos;

// Camera, scene and renderer
Camera* cam;
Scene scene;
MeshForwardRenderer* renderer;

// 3D objects
Mesh *city, *city2;
TextMesh *label;
Skybox* sky;

// Lights and shadow
PointLight *plight, *plight2;
DirectionalLight *dlight;

OBJLoader cityLoader;
bool loaderReady = false;

void setupModel();
void setupSkybox();
void render();

void keyDown(unsigned char ascii, int x, int y) {}

void loadTask() {
	cityLoader.load("models/city/The_City.obj", "models/city/");
	loaderReady = true;
}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.width = 1600;
	conf.height = 1200;
	conf.title = "Test: Loading Wavefront OBJ Model";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	ResourceFinder::get().add("../");
	ResourceFinder::get().add("../../");
	ResourceFinder::get().add("../../resources/");
	ResourceFinder::get().add("../../shaders/");

	// camera
	cam = new Camera(new PerspectiveLens(45.0f, static_cast<float>(conf.width) / static_cast<float>(conf.height), 0.1f, 1000.f));
	cam->move(glm::vec3(0, 0, 20));

	// renderer
	renderer = new MeshForwardRenderer;

	// 3d objects
	setupModel();
	setupSkybox();

	std::thread loadWorker(loadTask);

	// start the main loop
	Engine::start();

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
	label->setDoubleSided(true);

	/*
	//OBJLoader city2Loader("models/teapot/teapot.obj", "models/teapot/");
	OBJLoader city2Loader("models/street.obj", "models/");
	city2 = city2Loader.craftMesh(0, city2Loader.numGeometries(), "city2");
	city2->getTransform().appendScale(.2, .2, .2);
	city2->getTransform().appendMove(0, -40, -50);
	*/

	auto debug = new Mesh(new PlaneGeometry(5, 5), new ShadowTextureMaterial(renderer->getShadowMap()->getDebugTexture(0)));
	debug->getTransform().appendMove(30, 0, 10);

	scene.add(plight);
	scene.add(dlight);
	scene.add(label);
	scene.add(debug);

	renderer->getShadowMap()->setProjection(glm::ortho(-200.f, 200.f, -200.f, 100.f, -200.f, 500.f));
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

	scene.skybox = sky;
}

void render() {
	if (loaderReady) {
		loaderReady = false;
		city = cityLoader.craftMeshFromAllShapes();
		city->getTransform().appendScale(0.1f, 0.1f, 0.1f);
		city->getTransform().appendMove(0, -10, 0);

		scene.add(city);
		cityLoader.unload();
		label->setVisible(false);
	}

	float speedX = 0.1f, speedY = 0.1f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
	float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);

	//city2->getTransform().appendRotation(glm::radians(0.2), glm::vec3(0, 1, 0));

	// skybox
	renderer->render(&scene, cam);
}