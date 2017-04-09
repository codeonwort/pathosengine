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
#include <iostream>

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

void setupModel();
void setupSkybox();

void render() {
	float speedX = 0.05f, speedY = 0.05f;
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

void keyDown(unsigned char ascii, int x, int y) {}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.width = 800;
	conf.height = 600;
	conf.title = "Test: Loading Wavefront OBJ Model";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	// camera
	cam = new Camera(new PerspectiveLens(45.0f, 800.0f / 600.0f, 0.1f, 1000.f));
	cam->move(glm::vec3(0, 0, 20));

	// renderer
	renderer = new MeshForwardRenderer;

	// 3d objects
	setupModel();
	setupSkybox();

	// start the main loop
	Engine::start();

	return 0;
}

void setupModel() {
	plight = new PointLight(glm::vec3(5, 30, 5), glm::vec3(1, 1, 1));
	plight2 = new PointLight(glm::vec3(-15, 30, 5), glm::vec3(0, 0, 1));
	dlight = new DirectionalLight(glm::vec3(0.1, -1, 2), glm::vec3(1, 1, 1));

	OBJLoader cityLoader("../../resources/models/city/The_City.obj", "../../resources/models/city/");
	city = cityLoader.craftMesh(0, cityLoader.numGeometries(), "city");
	city->getTransform().appendScale(.1, .1, .1);
	city->getTransform().appendMove(0, -60, 0);

	label = new TextMesh("default");
	label->setText("text mesh test", 0xff0000);
	label->getTransform().appendScale(20);
	label->setDoubleSided(true);

	/*
	//OBJLoader city2Loader("../../resources/models/teapot/teapot.obj", "../../resources/models/teapot/");
	OBJLoader city2Loader("../../resources/models/street.obj", "../../resources/models/");
	city2 = city2Loader.craftMesh(0, city2Loader.numGeometries(), "city2");
	city2->getTransform().appendScale(.2, .2, .2);
	city2->getTransform().appendMove(0, -40, -50);
	*/

	auto debug = new Mesh(new PlaneGeometry(5, 5), new ShadowTextureMaterial(renderer->getShadowMap()->getDebugTexture(0)));
	debug->getTransform().appendMove(0, 0, 10);

	scene.add(plight);
	scene.add(dlight);
	scene.add(city);
	scene.add(label);
	scene.add(debug);

	renderer->getShadowMap()->setProjection(glm::ortho(-200.f, 200.f, -200.f, 100.f, -200.f, 500.f));
}

void setupSkybox() {
	const char* cubeImgName[6] = { "../../resources/cubemap1/pos_x.bmp", "../../resources/cubemap1/neg_x.bmp",
		"../../resources/cubemap1/pos_y.bmp", "../../resources/cubemap1/neg_y.bmp",
		"../../resources/cubemap1/pos_z.bmp", "../../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	scene.skybox = sky;
}