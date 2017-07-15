#include "pathos/engine.h"
#include "pathos/render/render_forward.h"
#include "pathos/render/render_norm.h"
#include "pathos/render/envmap.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/camera/camera.h"
#include "pathos/light/light.h"
#include "pathos/loader/imageloader.h"
#include "pathos/loader/objloader.h"

#include "glm/gtx/transform.hpp"
#include <iostream>

using namespace std;
using namespace pathos;

// camera
Camera* cam;

// scene
Scene scene;
	Mesh *cube, *viewer, *shadowLight; // shadow debugger
	Mesh *ball, *lamp; // shadow casters
	Mesh *plane_posX, *plane_negX, *plane_posY, *plane_negY, *plane_posZ, *plane_negZ; // shadow receivers
	Skybox* sky;

// renderer
MeshForwardRenderer* renderer;
NormalRenderer* normRenderer;

void setupScene();

void render() {
	float speedX = 0.05f, speedY = 0.05f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
	float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);
	lamp->getTransform().appendRotation(0.001f, glm::vec3(1.0f, 0.5f, 0.f));

	renderer->render(&scene, cam);
	normRenderer->render(ball, cam);
}

void keyDown(unsigned char ascii, int x, int y) {}

int main(int argc, char** argv) {
	// init the engine
	EngineConfig conf;
	conf.width = 800;
	conf.height = 600;
	conf.title = "Omnidirectional Light";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	cam = new Camera(new PerspectiveLens(45.0f, static_cast<float>(conf.width) / static_cast<float>(conf.height), 0.1f, 100.f));
	cam->lookAt(glm::vec3(0, 0, 30), glm::vec3(5, 0, 0), glm::vec3(0, 1, 0));

	renderer = new MeshForwardRenderer;
	normRenderer = new NormalRenderer(0.2);

	setupScene();

	// start the main loop
	Engine::start();

	return 0;
}

void setupScene() {
	// light and shadow
	auto light = new DirectionalLight(glm::vec3(0, -1, -0.1));
	auto plight = new PointLight(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1));
	auto plight2 = new PointLight(glm::vec3(-3, 5, 2), glm::vec3(1, 0, 0));
	auto plight3 = new PointLight(glm::vec3(2, -2, 0), glm::vec3(0, 1, 0));

	scene.add(light);
	scene.add(plight);
	//scene.add(plight2);
	//scene.add(plight3);

	// collada loader test
	/*ColladaLoader collada("../resources/birdcage2.dae");
	auto geoms = collada.getGeometries();
	lamp = new Mesh(nullptr, nullptr);
	lamp->getTransform().appendScale(0.5, 0.5, 0.5);
	auto testColor = make_shared<ColorMaterial>(1, 1, 1, 1);
	testColor->addLight(plight);
	for (auto geom : geoms){
	lamp->add(geom, testColor);
	}*/
	OBJLoader obj2("../../resources/models/lightbulb.obj", "../../resources/models/");
	lamp = obj2.craftMeshFromAllShapes();
	lamp->getTransform().appendScale(4, 4, 4);
	lamp->setDoubleSided(true);

	// cubemap
	const char* cubeImgName[6] = { "../../resources/cubemap1/pos_x.bmp", "../../resources/cubemap1/neg_x.bmp",
		"../../resources/cubemap1/pos_y.bmp", "../../resources/cubemap1/neg_y.bmp",
		"../../resources/cubemap1/pos_z.bmp", "../../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);
	scene.skybox = sky;

	// bump texture for planes
	GLuint tex = loadTexture(loadImage("../../resources/154.jpg"));
	GLuint tex_norm = loadTexture(loadImage("../../resources/154_norm.jpg"));
	auto mat = new BumpTextureMaterial(tex, tex_norm);

	auto planeGeom = new PlaneGeometry(30, 30);
	planeGeom->calculateTangentBasis();

	plane_posX = new Mesh(planeGeom, mat);
	plane_posX->getTransform().appendMove(15, 0, 0);
	plane_posX->getTransform().appendRotation(glm::radians(-90.0f), glm::vec3(0, 1, 0));

	plane_negX = new Mesh(planeGeom, mat);
	plane_negX->getTransform().appendMove(-15, 0, 0);
	plane_negX->getTransform().appendRotation(glm::radians(90.0f), glm::vec3(0, 1, 0));

	plane_posY = new Mesh(planeGeom, mat);
	plane_posY->getTransform().appendMove(0, 15, 0);
	plane_posY->getTransform().appendRotation(glm::radians(90.0f), glm::vec3(1, 0, 0));

	plane_negY = new Mesh(planeGeom, mat);
	plane_negY->getTransform().appendMove(0, -15, 0);
	plane_negY->getTransform().appendRotation(glm::radians(-90.0f), glm::vec3(1, 0, 0));

	plane_posZ = new Mesh(planeGeom, mat);
	plane_posZ->getTransform().appendMove(0, 0, 15);
	plane_posZ->getTransform().appendRotation(glm::radians(180.0f), glm::vec3(0, 1, 0));

	plane_negZ = new Mesh(planeGeom, mat);
	plane_negZ->getTransform().appendMove(0, 0, -15);

	// color material for ball
	auto color = new ColorMaterial;

	ball = new Mesh(new SphereGeometry(2, 40), color);
	ball->getTransform().appendMove(7, 4, 0);

	// omni shadow debugger
	auto omni = renderer->getOmnidirectionalShadow();
	unsigned int face = 0;
	viewer = new Mesh(new PlaneGeometry(10, 10), new ShadowCubeTextureMaterial(omni->getDebugTexture(0), face, omni->getLightNearZ(), omni->getLightFarZ()));
	viewer->getTransform().appendMove(10, 0, 10);
	viewer->setDoubleSided(true);

	cube = new Mesh(new CubeGeometry(glm::vec3(20, 20, 10)), new WireframeMaterial(1, 1, 1));
	GLfloat* lightDir = light->getDirection();
	//glm::vec3 lightPos = glm::vec3(-lightDir[0], -lightDir[1], -lightDir[2]) * 5.0f;
	glm::vec3 lightPos = plight->getPositionVector();
	cube->getTransform().append(glm::lookAt(lightPos, glm::vec3(0, 0, -lightPos.z), glm::vec3(0, 1, 0)));

	auto shadowLightColor = new ColorMaterial;
	shadowLightColor->setAmbient(1, 1, 0);
	shadowLightColor->setDiffuse(0, 0, 0);
	shadowLight = new Mesh(new SphereGeometry(0.3f, 20), shadowLightColor);
	shadowLight->getTransform().appendMove(lightPos);

	// scene configuration
	scene.skybox = sky;
	scene.add(ball);
	scene.add(lamp);
	scene.add(viewer);
	//scene.add(cube); // bounding box of shadow mapping
	scene.add(shadowLight);
	scene.add({ plane_posX, plane_negX, plane_posY, plane_negY, plane_posZ, plane_negZ });
}