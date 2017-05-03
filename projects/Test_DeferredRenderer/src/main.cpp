#include "pathos/engine.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/light/light.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include "pathos/render/render_norm.h"
#include "pathos/render/render_deferred.h"
#include "pathos/loader/imageloader.h"
#include "glm/gtx/transform.hpp"
#include <iostream>
#include <ctime>

using namespace std;
using namespace pathos;

// Camera, Scene, and renderer
Camera* cam;
Scene scene;
DeferredRenderer* renderer;
NormalRenderer* normRenderer;

// 3D objects
Mesh *model, *model2, *model3;
Skybox* sky;

// Lights and shadow
PointLight *plight;
DirectionalLight *dlight;

void setupScene();
void render();
void keyDown(unsigned char ascii, int x, int y) {}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.width = 800;
	conf.height = 600;
	conf.title = "Test: Deferred Rendering";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	// camera
	cam = new Camera(new PerspectiveLens(45.0f, (float)conf.width / conf.height, 0.1f, 1000.f));
	cam->move(glm::vec3(0, 0, 100));

	// renderer
	renderer = new DeferredRenderer;
	//normRenderer = new NormalRenderer(0.2);

	// scene
	setupScene();

	// start the main loop
	Engine::start();

	return 0;
}

void setupScene() {
	// light
	//plight = new PointLight(glm::vec3(0, 0, 20), glm::vec3(1, 1, 1));
	dlight = new DirectionalLight(glm::vec3(1, 0, 0), glm::vec3(0.1, 0.1, 0.1));
	scene.add(dlight);

	srand(time(NULL));
	for (int i = 0; i < 20; ++i) {
		float x = rand() % 50 - 25;
		float y = rand() % 50 - 25;
		float z = rand() % 50 - 25;
		float r = (rand() % 256) / 255.0;
		float g = (rand() % 256) / 255.0;
		float b = (rand() % 256) / 255.0;
		float power = 0.1 + 0.5 * (rand() % 100) / 100.0;
		scene.add(new PointLight(glm::vec3(x, y, z), glm::vec3(r, g, b)));
	}

	/*scene.add(new DirectionalLight(glm::vec3(0, -1, 0), glm::vec3(0.1, 0, 0.1)));
	scene.add(new DirectionalLight(glm::vec3(0.2, 0, -1), glm::vec3(0.0, 0.5, 1)));
	scene.add(new DirectionalLight(glm::vec3(-0.5, -0.5, 0), glm::vec3(0.0, 0.3, 0.4)));
	scene.add(new DirectionalLight(glm::vec3(0, 0.5, 0.5), glm::vec3(0.1, 0.3, 0.1)));
	scene.add(new DirectionalLight(glm::vec3(0, 1, 1), glm::vec3(0.0, 0.3, 1)));*/

	//---------------------------------------------------------------------------------------
	// create materials
	//---------------------------------------------------------------------------------------
	const char* cubeImgName[6] = { "../../resources/cubemap1/pos_x.bmp", "../../resources/cubemap1/neg_x.bmp",
		"../../resources/cubemap1/pos_y.bmp", "../../resources/cubemap1/neg_y.bmp",
		"../../resources/cubemap1/pos_z.bmp", "../../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTexture = loadCubemapTexture(cubeImg);

	GLuint tex = loadTexture(loadImage("../../resources/154.jpg"));
	GLuint tex_norm = loadTexture(loadImage("../../resources/154_norm.jpg"));

	auto material_texture = new TextureMaterial(tex);
	auto material_bump = new BumpTextureMaterial(tex, tex_norm);
	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAmbient(0.2f, 0.2f, 0.2f);
		color->setDiffuse(1.0f, 1.0f, 1.0f);
		color->setSpecular(1.0f, 1.0f, 1.0f);
		color->setAlpha(1.0f);
	}
	auto material_cubemap = new CubeEnvMapMaterial(cubeTexture);
	auto material_wireframe = new WireframeMaterial(0.0f, 1.0f, 1.0f, 0.3f);

	//---------------------------------------------------------------------------------------
	// create geometries
	//---------------------------------------------------------------------------------------
	
	auto geom_sphere_big = new SphereGeometry(15.0f, 30);
	auto geom_sphere = new SphereGeometry(5.0f, 30);
	auto geom_plane = new PlaneGeometry(10.f, 10.f);
	auto geom_cube = new CubeGeometry(glm::vec3(5.0f));

	geom_sphere->calculateTangentBasis();
	geom_sphere_big->calculateTangentBasis();
	geom_plane->calculateTangentBasis();
	geom_cube->calculateTangentBasis();


	//---------------------------------------------------------------------------------------
	// create meshes
	//---------------------------------------------------------------------------------------

	// skybox
	sky = new Skybox(cubeTexture);

	// model 1: flat texture
	model = new Mesh(geom_sphere_big, material_texture);
	model->getTransform().appendMove(-40, 0, 0);

	// model 2: solid color
	model2 = new Mesh(geom_sphere, material_color);
	model2->getTransform().appendMove(50, 0, 0);
	model2->getTransform().appendRotation(glm::radians(-10.f), glm::vec3(0, 1, 0));

	// model 3: wireframe
	model3 = new Mesh(geom_cube, material_wireframe);
	model3->getTransform().appendMove(30, 30, 10);

	// add them to scene
	scene.add(model);
	scene.add(model2);
	scene.add(model3);
	scene.skybox = sky;
}

void render() {
	float speedX = 0.1f, speedY = 0.1f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
	float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);

	model->getTransform().appendMove(0, 20, 0);
	model->getTransform().appendRotation(0.01f, glm::vec3(0, 0.5, 1));
	model->getTransform().appendMove(0, -20, 0);

	model2->getTransform().appendMove(-60, 0, 0);
	model2->getTransform().appendRotation(0.01f, glm::vec3(0, 1, 0));
	model2->getTransform().appendMove(60, 0, 0);

	renderer->render(&scene, cam);
	//normRenderer->render(model, cam);
}