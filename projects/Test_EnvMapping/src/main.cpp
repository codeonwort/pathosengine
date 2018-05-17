#include "glm/gtx/transform.hpp"
#include "pathos/engine.h"
#include "pathos/render/render_forward.h"
#include "pathos/render/render_norm.h"
#include "pathos/render/envmap.h"
#include "pathos/mesh/mesh.h"
#include "pathos/camera/camera.h"
#include "pathos/light/light.h"
#include "pathos/loader/imageloader.h"
#include "pathos/loader/objloader.h"
#include "pathos/text/textmesh.h"
#include "pathos/util/resource_finder.h"
#include <iostream>

using namespace std;
using namespace pathos;

// Rendering configurations
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float FOV = 90.0f;
const glm::vec3 CAMERA_POSITION(0.0f, 0.0f, 20.0f);
const char* TITLE = "Test: Envrionmental Mapping";

// Camera and scnee
Camera* cam;
Scene scene;

// renderers
MeshForwardRenderer* renderer;
NormalRenderer* normRenderer;

// 3D objects
MeshMaterial* envMapMaterial = nullptr;
Mesh *teapot;
TextMesh *label;
Skybox* sky;

OBJLoader teapotLoader;

// Lights and shadow
PointLight *plight, *plight2;

void setupScene();

void render() {
	float speedX = 0.1f, speedY = 0.1f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
	float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);

	renderer->render(&scene, cam);
	normRenderer->render(teapot, cam);
}

void keyDown(unsigned char ascii, int x, int y) {}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.width = WINDOW_WIDTH;
	conf.height = WINDOW_HEIGHT;
	conf.title = TITLE;
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	ResourceFinder::get().add("../");
	ResourceFinder::get().add("../../");
	ResourceFinder::get().add("../../shaders/");

	// camera
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, (float)conf.width / conf.height, 0.1f, 1000.f));
	cam->move(CAMERA_POSITION);

	// renderer
	renderer = new MeshForwardRenderer;
	normRenderer = new NormalRenderer(5);

	// 3d objects
	setupScene();

	// start the main loop
	Engine::start();

	return 0;
}

void setupScene() {
	// skybox
	const char* cubeImgName[6] = {
		"../../resources/cubemap1/pos_x.jpg", "../../resources/cubemap1/neg_x.jpg",
		"../../resources/cubemap1/pos_y.jpg", "../../resources/cubemap1/neg_y.jpg",
		"../../resources/cubemap1/pos_z.jpg", "../../resources/cubemap1/neg_z.jpg" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	envMapMaterial = new CubeEnvMapMaterial(cubeTex);

	// light and shadow
	plight = new PointLight(glm::vec3(5, 0, 25), glm::vec3(1, 1, 1));
	plight2 = new PointLight(glm::vec3(-15, 30, 5), glm::vec3(0, 0, 1));

	// text label
	label = new TextMesh("default");
	label->setText("Envrionmental mapping test", 0xffff00);
	label->getTransform().appendScale(10, 10, 10);
	label->getTransform().appendMove(0, -5, 0);

	// 3d object
	teapotLoader.load("../../resources/models/teapot/teapot.obj", "../../resources/models/teapot/");
	teapot = teapotLoader.craftMeshFromAllShapes();
	for (auto i = 0u; i < teapot->getMaterials().size(); ++i) {
		teapot->setMaterial(i, envMapMaterial);
	}
	teapot->getTransform().appendScale(0.2f, 0.2f, 0.2f);
	teapot->getTransform().appendMove(0, -40, -50);

	// construct scene
	scene.skybox = sky;
	scene.add(teapot);
	scene.add(label);
}
