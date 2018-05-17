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
#include "pathos/util/resource_finder.h"
#include "glm/gtx/transform.hpp"
using namespace pathos;

#include <iostream>
#include <time.h>

// Compile options
#define USE_NORMAL_RENDERER 0

// Rendering configurations
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
//const int WINDOW_WIDTH = 800;
//const int WINDOW_HEIGHT = 600;
const float FOV = 90.0f;
const glm::vec3 CAMERA_POSITION = glm::vec3(0, 0, 100);
const bool USE_HDR = true;
const unsigned int NUM_POINT_LIGHTS = 4;
const unsigned int NUM_BALLS = 10;

// Camera, Scene, and renderer
Camera* cam;
Scene scene;
DeferredRenderer* renderer;
NormalRenderer* normRenderer;

// 3D objects
Mesh *model, *model2, *model3;
std::vector<Mesh*> balls;
Mesh* godRaySource;
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
	conf.width = WINDOW_WIDTH;
	conf.height = WINDOW_HEIGHT;
	conf.title = "Test: Deferred Rendering";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	ResourceFinder::get().add("../");
	ResourceFinder::get().add("../../");
	ResourceFinder::get().add("../../shaders/");

	// camera
	float aspect_ratio = static_cast<float>(conf.width) / static_cast<float>(conf.height);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, aspect_ratio, 1.0f, 1000.f));
	cam->move(CAMERA_POSITION);

	// renderer
	renderer = new DeferredRenderer(conf.width, conf.height);
	renderer->setHDR(USE_HDR);
#if USE_NORMAL_RENDERER
	normRenderer = new NormalRenderer(0.2);
#endif

	// scene
	setupScene();

	// start the main loop
	Engine::start();

	return 0;
}

void setupScene() {
	// light
	dlight = new DirectionalLight(glm::vec3(0, -1, 0), glm::vec3(1.0, 1.0, 1.0));
	scene.add(dlight);

	srand((unsigned int)time(NULL));
	for (auto i = 0u; i < NUM_POINT_LIGHTS; ++i) {
		float x = rand() % 50 - 25.0f;
		float y = rand() % 50 - 25.0f;
		float z = rand() % 50 - 25.0f;
		float r = (rand() % 256) / 255.0f;
		float g = (rand() % 256) / 255.0f;
		float b = (rand() % 256) / 255.0f;
		float power = 0.1f + 0.5f * (rand() % 100) / 100.0f;
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
	const char* cubeImgName[6] = {
		"resources/cubemap1/pos_x.jpg", "resources/cubemap1/neg_x.jpg",
		"resources/cubemap1/pos_y.jpg", "resources/cubemap1/neg_y.jpg",
		"resources/cubemap1/pos_z.jpg", "resources/cubemap1/neg_z.jpg"
	};
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = pathos::loadImage(cubeImgName[i]);
	GLuint cubeTexture = pathos::loadCubemapTexture(cubeImg);

	GLuint tex = pathos::loadTexture(loadImage("resources/154.jpg"));
	GLuint tex_norm = pathos::loadTexture(loadImage("resources/154_norm.jpg"));

	GLuint tex_debug = renderer->debug_godRayTexture();
	auto material_tex_debug = new TextureMaterial(tex_debug);

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
	model->getTransform().appendMove(-40.0f, 0.0f, 0.0f);

	// model 2: solid color
	model2 = new Mesh(geom_sphere, material_color);
	model2->getTransform().appendMove(50.0f, 0.0f, 0.0f);
	model2->getTransform().appendRotation(glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// model 3: wireframe
	model3 = new Mesh(geom_cube, material_wireframe);
	//model3 = new Mesh(geom_plane, material_tex_debug);
	model3->getTransform().appendMove(5, 30, 0);

	// model: balls
	for (auto i = 0u; i < NUM_BALLS; ++i) {
		Mesh* ball = new Mesh(geom_sphere, material_bump);
		ball->getTransform().appendMove(0.0f, -10.0f, -15.0f * i);
		balls.push_back(ball);
		scene.add(ball);
	}

	// model: god ray source
	godRaySource = new Mesh(geom_sphere, material_color);
	godRaySource->getTransform().appendMove(0.0f, 300.0f, -500.0f);
	godRaySource->getTransform().appendScale(10.0f);

	// add them to scene
	scene.add(model);
	scene.add(model2);
	//scene.add(model3);
	//scene.add(godRaySource);
	scene.skybox = sky;
	scene.godRaySource = godRaySource;
}

void render() {
	float speedX = 0.5f, speedY = 0.5f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
	float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);

	model->getTransform().appendMove(0, 20, 0);
	model->getTransform().appendRotation(0.01f, glm::vec3(0.0f, 0.5f, 1.0f));
	model->getTransform().appendMove(0, -20, 0);

	model2->getTransform().appendMove(-60, 0, 0);
	model2->getTransform().appendRotation(0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
	model2->getTransform().appendMove(60, 0, 0);

	for (auto& ball : balls) {
		ball->getTransform().appendRotation(0.005f, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	renderer->render(&scene, cam);
#if USE_NORMAL_RENDERER
	normRenderer->render(model, cam);
	normRenderer->render(model2, cam);
#endif
}