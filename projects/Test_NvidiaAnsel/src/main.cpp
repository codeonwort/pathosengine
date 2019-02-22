#include "glm/gtx/transform.hpp"
#include "pathos/core_minimal.h"
#include "pathos/render/render_forward.h"
#include "pathos/mesh/mesh.h"
#include "pathos/camera/camera.h"
#include "pathos/light/light.h"
#include "pathos/loader/imageloader.h"
#include "pathos/loader/objloader.h"
#include "pathos/text/textmesh.h"
#include "pathos/mesh/geometry_primitive.h"

#include "ansel_envmap.h"

#include <thread>

using namespace pathos;

#define LOAD_3D_MODEL 1

// Rendering configurations
const int WINDOW_WIDTH          = 1920;
const int WINDOW_HEIGHT         = 1080;
const float FOV                 = 120.0f;
const glm::vec3 CAMERA_POSITION = glm::vec3(0.0f, 0.0f, 20.0f);

Camera* cam;
Scene scene;
	AnselEnvMapping* sky;
	Mesh *city, *city2;
	TextMesh *label;
	PointLight *plight, *plight2;
	DirectionalLight *dlight;

ForwardRenderer* renderer;

OBJLoader cityLoader;
bool loaderReady = false;

void setupModel();
void setupSkybox();
void tick();
void render();

void loadTask() {
	cityLoader.load("models/city/The_City.obj", "models/city/");
	loaderReady = true;
}

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = "Test: Nvidia Ansel";
	conf.tick         = tick;
	conf.render       = render;
	Engine::init(&argc, argv, conf);

	// camera
	const float aspectRatio = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, aspectRatio, 1.0f, 1000.f));
	cam->move(CAMERA_POSITION);

	// renderer
	renderer = new ForwardRenderer;

	// 3d objects
	setupModel();
	setupSkybox();

#if LOAD_3D_MODEL
	std::thread loadWorker(loadTask);
#endif

	// start the main loop
	gEngine->start();

#if LOAD_3D_MODEL
	loadWorker.join();
#endif

	return 0;
}

void setupModel() {
	plight = new PointLight(glm::vec3(5, 30, 5), glm::vec3(1, 1, 1));
	plight2 = new PointLight(glm::vec3(-15, 30, 5), glm::vec3(0, 0, 1));
	dlight = new DirectionalLight(glm::vec3(0.1, -1, 2), glm::vec3(1, 1, 1));

#if LOAD_3D_MODEL
	label = new TextMesh("default");
	label->setText("Loading OBJ model. Please wait...", 0xff0000);
	label->getTransform().appendScale(20);
	label->doubleSided = true;

	auto debug = new Mesh(new PlaneGeometry(5, 5), new ShadowTextureMaterial(renderer->getShadowMap()->getDebugTexture(0)));
	debug->getTransform().appendMove(30, 0, 10);
#endif

	scene.add(plight);
	scene.add(dlight);
#if LOAD_3D_MODEL
	scene.add(label);
	scene.add(debug);
#endif

	renderer->getShadowMap()->setProjection(glm::ortho(-200.f, 200.f, -200.f, 100.f, -200.f, 500.f));
}

void setupSkybox() {
	//auto anselImage = pathos::loadImage("ansel/dishonored.jpg");
	auto anselImage = pathos::loadImage("ansel/the_surge.jpg");
	GLuint anselTex = loadTexture(anselImage);
	sky = new AnselEnvMapping(anselTex);
	scene.skybox = nullptr; // use custom sky rendering
}

void tick() {
	if (loaderReady) {
		loaderReady = false;
		city = cityLoader.craftMeshFromAllShapes();
		city->getTransform().appendScale(0.1f, 0.1f, 0.1f);
		city->getTransform().appendMove(0.0f, -60.0f, 0.0f);

		scene.add(city);
		cityLoader.unload();
		label->visible = false;
	}

	if (gConsole->isVisible() == false) {
		float speedX = 0.1f, speedY = 0.1f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -1.0f : gEngine->isDown('e') ? 1.0f : 0.0f;
		float rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}
}

void render() {
	glm::mat4& view = glm::mat4(glm::mat3(cam->getViewMatrix())); // view transform without transition
	glm::mat4& proj = cam->getProjectionMatrix();
	glm::mat4& transform = proj * view;
	sky->render(transform);

	renderer->render(&scene, cam);
}
