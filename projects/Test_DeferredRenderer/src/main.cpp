#include "pathos/engine.h"
#include "pathos/console.h"
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

#include <GL/glut.h>
#include <iostream>
#include <time.h>

// Compile options
#define USE_NORMAL_RENDERER 0

// Rendering configurations
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const char* WINDOW_TITLE = "Test: Deferred Rendering";
const float FOV = 90.0f;
const glm::vec3 CAMERA_POSITION = glm::vec3(0, 0, 100);
const bool USE_HDR = true;
const unsigned int NUM_POINT_LIGHTS = 2;
const unsigned int NUM_BALLS = 10;

// Console window
ConsoleWindow* g_Console = nullptr;

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

// Profiler
GLuint timer_query = 0;

// Lights and shadow
PointLight *plight;
DirectionalLight *dlight;

void prepareProfiler();
void setupScene();
void render();
void keyDown(unsigned char ascii, int x, int y) {}

void keyPress(unsigned char ascii) {
	// backtick
	if (ascii == 0x60) {
		g_Console->toggle();
	} else if (g_Console->isVisible()) {
		g_Console->onKeyPress(ascii);
	}
}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.width = WINDOW_WIDTH;
	conf.height = WINDOW_HEIGHT;
	conf.title = WINDOW_TITLE;
	conf.render = render;
	conf.keyDown = keyDown;
	conf.keyPress = keyPress;
	Engine::init(&argc, argv, conf);

	ResourceFinder::get().add("../");
	ResourceFinder::get().add("../../");
	ResourceFinder::get().add("../../shaders/");

	// console
	g_Console = new ConsoleWindow;
	if (g_Console->initialize(WINDOW_WIDTH, 400) == false) {
		std::cerr << "Failed to initialize console window" << std::endl;
		delete g_Console;
		g_Console = nullptr;
	}
	g_Console->addLine(L"Built-in debug console. Press ` to toggle.");

	// camera
	float aspect_ratio = static_cast<float>(conf.width) / static_cast<float>(conf.height);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, aspect_ratio, 1.0f, 1000.f));
	cam->move(CAMERA_POSITION);

	// renderer
	renderer = new DeferredRenderer(conf.width, conf.height);
	renderer->setHDR(USE_HDR);
#if USE_NORMAL_RENDERER
	normRenderer = new NormalRenderer(0.2f);
#endif


	// scene
	setupScene();

	// profiler
	prepareProfiler();

	// start the main loop
	Engine::start();

	return 0;
}

void setupScene() {
	// light
	dlight = new DirectionalLight(glm::vec3(0, -1, 0), 1.0f * glm::vec3(1.0, 1.0, 1.0));
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
	auto material_wireframe = new WireframeMaterial(1.0f, 0.0f, 1.0f, 1.0f);

	// PBR material
	PBRTextureMaterial* material_pbr;
	{
#if 1
		GLuint albedo		= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-albedo.png"));
		GLuint normal		= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-normal-ue.png"));
		GLuint metallic		= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-metalness.png"));
		GLuint roughness	= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-roughness.png"));
		GLuint ao			= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-ao.png"));
#else
		GLuint albedo		= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-albedo.png"));
		GLuint normal		= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-normal.png"));
		GLuint metallic		= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-metalness.png"));
		GLuint roughness	= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-rough.png"));
		GLuint ao			= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-ao.png"));
#endif
		material_pbr = new PBRTextureMaterial(albedo, normal, metallic, roughness, ao);
	}
	

	//---------------------------------------------------------------------------------------
	// create geometries
	//---------------------------------------------------------------------------------------
	
	auto geom_sphere_big	= new SphereGeometry(15.0f, 30);
	auto geom_sphere		= new SphereGeometry(5.0f, 30);
	auto geom_plane			= new PlaneGeometry(10.0f, 10.0f);
	auto geom_cube			= new CubeGeometry(glm::vec3(5.0f));

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
	//model = new Mesh(geom_sphere_big, material_texture);
	model = new Mesh(geom_plane, material_bump);
	model->getTransform().appendMove(-40.0f, 0.0f, 0.0f);

	// model 2: solid color
	model2 = new Mesh(geom_sphere, material_color);
	model2->getTransform().appendRotation(glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model2->getTransform().appendMove(50.0f, 0.0f, 0.0f);

	// model 3: wireframe
	model3 = new Mesh(geom_cube, material_wireframe);
	//model3 = new Mesh(geom_plane, material_tex_debug);
	model3->getTransform().appendMove(35.0f, 0.0f, 0.0f);

	// model: balls
	for (auto i = 0u; i < NUM_BALLS; ++i) {
		Mesh* ball = new Mesh(geom_sphere, material_pbr);
		ball->getTransform().appendMove(0.0f, -10.0f, -15.0f * i);
		balls.push_back(ball);
		scene.add(ball);
	}

	// model: god ray source
	godRaySource = new Mesh(geom_sphere, material_color);
	godRaySource->getTransform().appendScale(10.0f);
	godRaySource->getTransform().appendMove(0.0f, 300.0f, -500.0f);

	// add them to scene
	scene.add(model);
	scene.add(model2);
	scene.add(model3);
	scene.skybox = sky;
	scene.godRaySource = godRaySource;
}

void prepareProfiler() {
	glGenQueries(1, &timer_query);
	assert(timer_query != 0);
}

void render() {
	if (g_Console->isVisible() == false) {
		float speedX = 0.5f, speedY = 0.5f;
		float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
		float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
		float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
		float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}

	/*
	model->getTransform().appendMove(0, 20, 0);
	model->getTransform().appendRotation(0.01f, glm::vec3(0.0f, 0.5f, 1.0f));
	model->getTransform().appendMove(0, -20, 0);
	*/

	model2->getTransform().appendMove(-60, 0, 0);
	model2->getTransform().appendRotation(0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
	model2->getTransform().appendMove(60, 0, 0);

	for (auto& ball : balls) {
		ball->getTransform().prependRotation(0.002f, glm::vec3(0.0f, 1.0f, 1.0f));
	}

	// Start timer
	GLuint64 elapsed_ms;
	GLuint64 elapsed_ns;
	glBeginQuery(GL_TIME_ELAPSED, timer_query);

	// Render
	renderer->render(&scene, cam);

	// End timer
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectui64v(timer_query, GL_QUERY_RESULT, &elapsed_ns);
	elapsed_ms = elapsed_ns / 1000000u;

	char title[256];
	sprintf_s(title, "%s (Elapsed: %llu ms)", WINDOW_TITLE, elapsed_ms);
	glutSetWindowTitle(title);

#if USE_NORMAL_RENDERER
	for (const auto mesh : scene.meshes) {
		normRenderer->render(mesh, cam);
	}
#endif

	if (g_Console) {
		g_Console->render();
	}
}