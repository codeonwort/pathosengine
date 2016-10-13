#include <iostream>

#include <glm/gtx/transform.hpp>
#include <pathos/engine.h>
#include <pathos/render/render.h>
#include <pathos/mesh/mesh.h>
#include <pathos/mesh/envmap.h>
#include <pathos/camera/camera.h>
#include <pathos/light/light.h>
#include <pathos/loader/imageloader.h>
#include <pathos/loader/objloader.h>
#include <pathos/text/textmesh.h>

using namespace std;
using namespace pathos;

// Camera and renderer
Camera* cam;
MeshDefaultRenderer* renderer;

// 3D objects
Mesh *city, *city2;
TextMesh *label;
Skybox* sky;

// Lights and shadow
PointLight *plight, *plight2;
OmnidirectionalShadow* shadow;

void setupModel();
void setupPlanes();
void setupSkybox();

void render() {
	float speedX = 0.1f, speedY = 0.1f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float dr = Engine::isDown('q') ? 0.5f : Engine::isDown('e') ? -0.5f : 0.0f;
	float dr2 = Engine::isDown('z') ? 0.2f : Engine::isDown('x') ? -0.2f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateZ(dr);
	cam->rotateX(dr2);
	//cam->rotate(dr, glm::vec3(0, 1, 0));
	//cam->rotate(dr2, glm::vec3(1, 0, 0));

	//city2->getTransform().appendRotation(glm::radians(0.2), glm::vec3(0, 1, 0));
	
	renderer->ready();

	// skybox
	renderer->render(sky, cam);
	
	// various models
	//renderer->render(city, cam);
	renderer->render(city2, cam);
	renderer->render(label, cam);
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
	renderer = new MeshDefaultRenderer();

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
	shadow = new OmnidirectionalShadow(plight, cam);

	/*OBJLoader cityLoader("../../resources/city/The_City.obj", "../../resources/city/");
	city = cityLoader.craftMesh(0, cityLoader.numGeometries(), "city");
	city->getTransform().appendScale(.1, .1, .1);
	for (int i = 0; i < cityLoader.getMaterials().size(); i++){
		auto& mat = cityLoader.getMaterials()[i];
		mat->addLight(plight);
		mat->addLight(plight2);
		mat->setShadowMethod(shadow);
	}
	city->getTransform().appendMove(0, -60, 0);*/

	label = new TextMesh("default");
	label->setText("text mesh test", 0xff0000);
	label->getTransform().appendScale(10, 10, 10);
	label->setDoubleSided(true);

	//OBJLoader city2Loader("../../resources/models/teapot/teapot.obj", "../../resources/models/teapot/");
	OBJLoader city2Loader("../../resources/models/street.obj", "../../resources/models/");
	for (int i = 0; i < city2Loader.getMaterials().size(); i++){
		auto& mat = city2Loader.getMaterials()[i];
		mat->addLight(plight);
		//mat->addLight(plight2);
		//mat->setShadowMethod(shadow);
	}
	city2 = city2Loader.craftMesh(0, city2Loader.numGeometries(), "city2");
	city2->getTransform().appendScale(.2, .2, .2);
	city2->getTransform().appendMove(0, -40, -50);
	//city2->setDoubleSided(true);
}

void setupSkybox() {
	const char* cubeImgName[6] = { "../../resources/cubemap1/pos_x.bmp", "../../resources/cubemap1/neg_x.bmp",
		"../../resources/cubemap1/pos_y.bmp", "../../resources/cubemap1/neg_y.bmp",
		"../../resources/cubemap1/pos_z.bmp", "../../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);
}