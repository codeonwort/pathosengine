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

using namespace std;
using namespace pathos;

Camera* cam;
Mesh *cube, *viewer, *shadowLight; // shadow debugger
Mesh *ball, *lamp; // shadow casters
Mesh *plane_posX, *plane_negX, *plane_posY, *plane_negY, *plane_posZ, *plane_negZ; // shadow receivers
Mesh *car; // assimp test (incomplete)
Skybox* sky;
MeshDefaultRenderer* renderer;

void render() {
	float speedX = 0.1f, speedY = 0.1f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float dr = Engine::isDown('q') ? 0.5f : Engine::isDown('e') ? -0.5f : 0.0f;
	float dr2 = Engine::isDown('z') ? 0.5f : Engine::isDown('x') ? -0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotate(dr, glm::vec3(0, 1, 0));
	cam->rotate(dr2, glm::vec3(1, 0, 0));
	lamp->getTransform().appendRotation(0.001f, glm::vec3(1.0f, 0.5f, 0.f));

	renderer->ready();
	// various models
	renderer->render(ball, cam);
	renderer->render(lamp, cam);
	// shadow debugger
	//renderer->render(viewer, cam);
	//renderer->render(cube, cam);
	renderer->render(shadowLight, cam);
	renderer->render(plane_posX, cam);
	renderer->render(plane_negX, cam);
	renderer->render(plane_posY, cam);
	renderer->render(plane_negY, cam);
	renderer->render(plane_posZ, cam);
	renderer->render(plane_negZ, cam);
	// skybox
	renderer->render(sky, cam);
}

void keyDown(unsigned char ascii, int x, int y) {}

int main(int argc, char** argv) {
	// init the engine
	EngineConfig conf;
	conf.width = 800;
	conf.height = 600;
	conf.title = "engine test";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	cam = new Camera(new PerspectiveLens(45.0f, 800.0f / 600.0f, 0.1f, 100.f));
	cam->move(glm::vec3(-0.2, 0, 3));

	// light and shadow
	auto light = new DirectionalLight(glm::vec3(0, -1, -0.1));
	//auto shadow = new ShadowMap(light, cam);
	auto plight = new PointLight(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1));
	auto plight2 = new PointLight(glm::vec3(13, 10, 5), glm::vec3(1, 0, 0));
	auto plight3 = new PointLight(glm::vec3(20, -10, 13), glm::vec3(0, 1, 0));
	auto shadow = new OmnidirectionalShadow(plight, cam);

	// obj loader test
	//OBJLoader obj("../resources/volkswagen/Volkswagen.obj", "../resources/volkswagen/");
	OBJLoader obj("../../resources/models/Pin.obj", "../../models/resources/");
	car = obj.craftMesh(0, obj.numGeometries(), "car");
	car->getTransform().appendMove(7, 1, 0);
	car->getTransform().appendScale(20, 20, 20);
	for (auto M : obj.getMaterials()){
		M->addLight(plight);
		M->addLight(plight2);
		M->addLight(plight3);
		M->setShadowMethod(shadow);
	}

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
	lamp = obj2.craftMesh(0, obj2.numGeometries(), "lamp");
	lamp->getTransform().appendScale(4, 4, 4);
	lamp->getMaterials()[0]->addLight(plight);
	lamp->getMaterials()[0]->addLight(plight2);
	lamp->getMaterials()[0]->setShadowMethod(shadow);
	lamp->setDoubleSided(true);

	// cubemap
	const char* cubeImgName[6] = { "../../resources/cubemap1/pos_x.bmp", "../../resources/cubemap1/neg_x.bmp",
		"../../resources/cubemap1/pos_y.bmp", "../../resources/cubemap1/neg_y.bmp",
		"../../resources/cubemap1/pos_z.bmp", "../../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	auto color = make_shared<ColorMaterial>(1.0, 1.0, 1.0, 1);
	//color->setAmbientColor(0, 0, 0);
	color->setSpecularColor(1, 1, 1);
	color->addLight(plight);
	color->addLight(plight2);
	color->addLight(plight3);
	color->setShadowMethod(shadow);

	GLuint tex = loadTexture(loadImage("../../resources/154.jpg"));
	GLuint tex_norm = loadTexture(loadImage("../../resources/154_norm.jpg"));
	auto mat = make_shared<BumpTextureMaterial>(tex, tex_norm, plight);
	mat->setShadowMethod(shadow);
	
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

	ball = new Mesh(new SphereGeometry(2, 40), color);
	//ball->getGeometries()[0]->calculateNormals();
	ball->getTransform().appendMove(7, 4, 0);

	viewer = new Mesh(new PlaneGeometry(3, 3), make_shared<ShadowCubeTextureMaterial>(shadow->getDebugTexture(), 0));
	//viewer = new Mesh(new PlaneGeometry(3, 3), make_shared<ShadowTextureMaterial>(shadow->getDebugTexture()));
	viewer->getTransform().appendMove(10, 0, 0);

	cube = new Mesh(new CubeGeometry(glm::vec3(20, 20, 10)), make_shared<WireframeMaterial>(1, 1, 1));
	GLfloat* lightDir = light->getDirection();
	//glm::vec3 lightPos = glm::vec3(-lightDir[0], -lightDir[1], -lightDir[2]) * 5.0f;
	glm::vec3 lightPos = plight->getPositionVector();
	cube->getTransform().append(glm::lookAt(lightPos, glm::vec3(0, 0, -lightPos.z), glm::vec3(0, 1, 0)));

	auto shadowLightColor = make_shared<ColorMaterial>(0, 0, 0, 1);
	shadowLightColor->setAmbientColor(1, 1, 0);
	shadowLight = new Mesh(new SphereGeometry(0.3f, 20), shadowLightColor);
	shadowLight->getTransform().appendMove(lightPos);

	renderer = new MeshDefaultRenderer();

	// start the main loop
	Engine::start();

	return 0;
}