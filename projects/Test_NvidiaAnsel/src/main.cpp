#include "glm/gtx/transform.hpp"
#include "pathos/engine.h"
#include "pathos/render/render_forward.h"
#include "pathos/mesh/mesh.h"
#include "pathos/camera/camera.h"
#include "pathos/light/light.h"
#include "pathos/loader/imageloader.h"
#include "pathos/loader/objloader.h"
#include "pathos/text/textmesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/resource_finder.h"

#include "ansel_envmap.h"

#include <iostream>
#include <thread>

using namespace std;
using namespace pathos;

// Rendering configurations
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float FOV = 90.0f;
const glm::vec3 CAMERA_POSITION(0.0f, 0.0f, 20.0f);

// Camera, scene and renderer
Camera* cam;
Scene scene;
MeshForwardRenderer* renderer;

// 3D objects
Mesh *city, *city2;
TextMesh *label;
AnselEnvMapping* sky;

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
	conf.width = WINDOW_WIDTH;
	conf.height = WINDOW_HEIGHT;
	conf.title = "Test: Nvidia Ansel";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	ResourceFinder::get().add("../");
	ResourceFinder::get().add("../../");
	ResourceFinder::get().add("../../resources/");

	// camera
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, static_cast<float>(conf.width) / static_cast<float>(conf.height), 1.0f, 500.f));
	cam->move(CAMERA_POSITION);

	// renderer
	renderer = new MeshForwardRenderer;

	// 3d objects
	setupModel();
	setupSkybox();

	//std::thread loadWorker(loadTask);

	// start the main loop
	Engine::start();

	//loadWorker.join();

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

	auto debug = new Mesh(new PlaneGeometry(5, 5), new ShadowTextureMaterial(renderer->getShadowMap()->getDebugTexture(0)));
	debug->getTransform().appendMove(30, 0, 10);

	scene.add(plight);
	scene.add(dlight);
	//scene.add(label);
	scene.add(debug);

	renderer->getShadowMap()->setProjection(glm::ortho(-200.f, 200.f, -200.f, 100.f, -200.f, 500.f));
}

void setupSkybox() {
	auto anselImage = pathos::loadImage("ansel/dishonored.jpg");
	//auto anselImage = pathos::loadImage("ansel/the_surge.jpg");
	GLuint anselTex = loadTexture(anselImage);
	sky = new AnselEnvMapping(anselTex);
	scene.skybox = nullptr; // use custom sky rendering
}

void render() {
	if (loaderReady) {
		loaderReady = false;
		city = cityLoader.craftMeshFromAllShapes();
		city->getTransform().appendScale(0.1f, 0.1f, 0.1f);
		city->getTransform().appendMove(0.0f, -60.0f, 0.0f);

		//scene.add(city);
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

	glm::mat4& view = glm::mat4(glm::mat3(cam->getViewMatrix())); // view transform without transition
	glm::mat4& proj = cam->getProjectionMatrix();
	glm::mat4& transform = proj * view;
	sky->render(transform);

	renderer->render(&scene, cam);
}