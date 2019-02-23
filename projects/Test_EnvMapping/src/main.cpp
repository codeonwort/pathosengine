#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/text/textmesh.h"
#include <iostream>

using namespace pathos;
using namespace std;

const int WINDOW_WIDTH             =    1920;
const int WINDOW_HEIGHT            =    1080;
const float FOV                    =    90.0f;
const glm::vec3 CAMERA_POSITION    =    glm::vec3(0.0f, 0.0f, 20.0f);
const char* TITLE                  =    "Test: Envrionmental Mapping";

Camera* cam;
Scene scene;
	MeshMaterial* envMapMaterial = nullptr;
	Mesh*         teapot;
	TextMesh*     label;
	Skybox*       sky;

OBJLoader teapotLoader;

// Lights and shadow
PointLight *plight;
PointLight *plight2;

void setupScene();

void tick() {
	if (gConsole->isVisible() == false) {
		float speedX = 0.4f, speedY = 0.4f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -0.5f : gEngine->isDown('e') ? 0.5f : 0.0f;
		float rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}
}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = TITLE;
	conf.rendererType = ERendererType::Forward;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	const float aspectRatio = (float)conf.windowWidth / conf.windowHeight;
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, aspectRatio, 1.0f, 1000.f));
	cam->move(CAMERA_POSITION);

	setupScene();

	gEngine->setWorld(&scene, cam);
	gEngine->start();

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
	label->getTransform().appendMove(0, 0, 10);

	// 3d object
	teapotLoader.load("../../resources/models/teapot/teapot.obj", "../../resources/models/teapot/");
	teapot = teapotLoader.craftMeshFromAllShapes();
	for (auto i = 0u; i < teapot->getMaterials().size(); ++i) {
		teapot->setMaterial(i, envMapMaterial);
	}
	teapot->getTransform().appendScale(0.2f, 0.2f, 0.2f);
	teapot->getTransform().appendMove(0, -10, -30.0f);

	// construct scene
	scene.sky = sky;
	scene.add(teapot);
	scene.add(label);
}
