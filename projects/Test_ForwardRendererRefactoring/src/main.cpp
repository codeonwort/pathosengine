#include "pathos/core_minimal.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/light/light.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"
#include "pathos/render/render_forward.h"
#include "pathos/render/render_norm.h"
#include "pathos/loader/imageloader.h"
#include "pathos/util/resource_finder.h"
#include "glm/gtx/transform.hpp"

using namespace pathos;

// Configurations
constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 900;
constexpr char* WINDOW_TITLE = "Test: Refactor Forward Renderer";
constexpr float FOV = 90.0f;

// Camera, Scene, and renderer
Camera* cam;
Scene scene;
ForwardRenderer* renderer;
NormalRenderer* normRenderer;

// 3D objects
Mesh *model, *model2, *model3;
Mesh *model_shadowdebug;
Skybox* sky;

// Lights and shadow
PointLight *plight;
DirectionalLight *dlight;
//OmnidirectionalShadow* shadow;

void setupScene();
void tick();
void render();

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = WINDOW_TITLE;
	conf.tick         = tick;
	conf.render       = render;
	Engine::init(&argc, argv, conf);

	// camera
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, (float)conf.windowWidth / conf.windowHeight, 1.0f, 1000.f));
	cam->move(glm::vec3(0.0f, 0.0f, 50.0f));

	// renderer
	renderer = new ForwardRenderer;
	normRenderer = new NormalRenderer(0.2f);

	// scene
	setupScene();

	//GLint texture_bind_limit;
	//glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_bind_limit);
	//LOG(LogInfo, "GL_MAX_TEXTURE_IMAGE_UNITS: %d", texture_bind_limit);

	// start the main loop
	gEngine->start();

	return 0;
}

void setupScene() {
	// light
	plight = new PointLight(glm::vec3(-30, 0, 0), glm::vec3(1, 1, 1));
	dlight = new DirectionalLight(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));

	//---------------------------------------------------------------------------------------
	// create materials
	//---------------------------------------------------------------------------------------
	const char* cubeImgName[6] = {
		"cubemap1/pos_x.jpg", "cubemap1/neg_x.jpg",
		"cubemap1/pos_y.jpg", "cubemap1/neg_y.jpg",
		"cubemap1/pos_z.jpg", "cubemap1/neg_z.jpg"
	};
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTexture = loadCubemapTexture(cubeImg);

	GLuint tex = loadTexture(loadImage("154.jpg"));
	GLuint tex_norm = loadTexture(loadImage("154_norm.jpg"));

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

	GLint shadowTexture = renderer->getShadowMap()->getDebugTexture(0);
	auto material_shadowdebug = new ShadowTextureMaterial(shadowTexture);

	//---------------------------------------------------------------------------------------
	// create geometries
	//---------------------------------------------------------------------------------------
	
	auto geom_sphere_big = new SphereGeometry(15.0f, 30);
	auto geom_sphere = new SphereGeometry(5.0f);
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

	// model 1: solid color
	model = new Mesh(geom_sphere_big, material_bump);
	model->getTransform().appendMove(60, 0, 0);

	// model 2: flat texture
	model2 = new Mesh(geom_sphere, material_color);
	model2->getTransform().appendRotation(glm::radians(-10.f), glm::vec3(0, 1, 0));
	model2->getTransform().appendMove(30, 0, 0);

	// model 3: wireframe
	model3 = new Mesh(geom_cube, material_wireframe);
	model3->getTransform().appendMove(60, 30, 0);

	// model : shadow debug
	model_shadowdebug = new Mesh(geom_plane, material_shadowdebug);
	model_shadowdebug->getTransform().appendMove(5, 0, 40);
	model_shadowdebug->doubleSided = true;

	// add them to scene
	scene.add(model);
	scene.add(model2);
	scene.add(model3);
	scene.add(model_shadowdebug);
	scene.add(plight);
	scene.add(dlight);
	scene.skybox = sky;
}

void tick() {
	float speedX = 0.2f, speedY = 0.2f;
	float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
	float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
	float rotY = gEngine->isDown('q') ? -0.5f : gEngine->isDown('e') ? 0.5f : 0.0f;
	float rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);

	model2->getTransform().appendMove(30, 0, 0);
	model2->getTransform().appendRotation(0.01f, glm::vec3(0, 1, 0));
	model2->getTransform().appendMove(-30, 0, 0);
}

void render() {
	renderer->render(&scene, cam);
	normRenderer->render(model, cam);
	normRenderer->render(model2, cam);
}