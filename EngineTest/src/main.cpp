#include <iostream>
#include <pathos/engine.h>
#include <pathos/mesh/mesh.h>
#include <pathos/mesh/render.h>
#include <pathos/mesh/envmap.h>
#include <pathos/camera/camera.h>
#include <pathos/light/light.h>
#include <pathos/loader/imageloader.h>
#include <pathos/loader/objloader.h>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace pathos;

Camera* cam;
Mesh *cube, *car;
Mesh *caster, *viewer, *shadowLight;
Mesh *plane_posX, *plane_negX, *plane_posY, *plane_negY, *plane_posZ, *plane_negZ;
Skybox* sky;
Mesh* daeMesh;
MeshDefaultRenderer* renderer;

void render() {
	float dx = Engine::isDown('a') ? -0.05f : Engine::isDown('d') ? 0.05f : 0.0f;
	float dz = Engine::isDown('w') ? -0.05f : Engine::isDown('s') ? 0.05f : 0.0f;
	float dr = Engine::isDown('q') ? 0.5f : Engine::isDown('e') ? -0.5f : 0.0f;
	float dr2 = Engine::isDown('z') ? 0.5f : Engine::isDown('x') ? -0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotate(dr, glm::vec3(0, 1, 0));
	cam->rotate(dr2, glm::vec3(1, 0, 0));
	daeMesh->getTransform().appendRotation(0.001, glm::vec3(1.0f, 0.5f, 0));

	renderer->ready();
	// various models
	renderer->render(caster, cam);
	//renderer->render(car, cam);
	renderer->render(daeMesh, cam);
	// about shadow
	renderer->render(viewer, cam);
	renderer->render(cube, cam);
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

	cam = new Camera(new PerspectiveLens(45.0, 800.0 / 600.0, 0.1, 100.0));
	cam->move(glm::vec3(-0.2, 0, 3));
	//cam->rotate(20, glm::vec3(0, 1, 0));

	// light and shadow
	auto light = new DirectionalLight(glm::vec3(0, -1, -0.1));
	//auto shadow = new ShadowMap(light, cam);
	//auto plight = new PointLight(glm::vec3(0, 5, 5), glm::vec3(1, 1, 1));
	auto plight = new PointLight(glm::vec3(0, 2, 0), glm::vec3(1, 1, 1));
	auto plight2 = new PointLight(glm::vec3(5, 0, 5), glm::vec3(1, 0, 0));
	auto plight3 = new PointLight(glm::vec3(-5, -2, 3), glm::vec3(0, 1, 0));
	auto shadow = new OmnidirectionalShadow(plight, cam);

	// obj loader test
	//OBJLoader obj("../resources/volkswagen/Volkswagen.obj", "../resources/volkswagen/");
	OBJLoader obj("../resources/Pin.obj", "../resources/");
	car = obj.craftMesh(0, obj.numGeometries(), "car");
	car->getTransform().appendMove(7, 1, 0);
	car->getTransform().appendScale(20, 20, 20);
	for (auto M : car->getMaterials()){
		if (M->getDirectionalLights().size() == 0){
			M->addLight(plight);
			M->addLight(plight2);
			M->addLight(plight3);
			M->setShadowMethod(shadow);
		}
	}

	// collada loader test
	/*ColladaLoader collada("../resources/birdcage2.dae");
	auto geoms = collada.getGeometries();
	daeMesh = new Mesh(nullptr, nullptr);
	daeMesh->getTransform().appendScale(0.5, 0.5, 0.5);
	auto testColor = make_shared<ColorMaterial>(1, 1, 1, 1);
	testColor->addLight(plight);
	for (auto geom : geoms){
		daeMesh->add(geom, testColor);
	}*/
	OBJLoader obj2("../resources/lightbulb.obj", "../resources/");
	daeMesh = obj2.craftMesh(0, obj2.numGeometries(), "cage");
	daeMesh->getTransform().appendScale(4, 4, 4);
	daeMesh->getMaterials()[0]->addLight(plight);
	daeMesh->getMaterials()[0]->addLight(plight2);
	daeMesh->getMaterials()[0]->setShadowMethod(shadow);
	daeMesh->setDoubleSided(true);
	
	// cubemap
	const char* cubeImgName[6] = { "../resources/cubemap1/pos_x.bmp", "../resources/cubemap1/neg_x.bmp",
		"../resources/cubemap1/pos_y.bmp", "../resources/cubemap1/neg_y.bmp",
		"../resources/cubemap1/pos_z.bmp", "../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	auto color = make_shared<ColorMaterial>(1.0, 1.0, 1.0, 1);
	//color->setAmbientColor(0, 0, 0);
	color->addLight(plight);
	color->addLight(plight2);
	color->addLight(plight3);
	color->setShadowMethod(shadow);

	//GLuint tex = loadTexture(loadImage("../resources/image2.jpg"));
	GLuint tex = loadTexture(loadImage("../resources/151.jpg"));
	GLuint tex_norm = loadTexture(loadImage("../resources/151_norm.jpg"));
	auto mat = make_shared<TextureMaterial>(tex);
	//auto mat = make_shared<BumpTextureMaterial>(tex, tex_norm, plight);
	//mat->addLight(light);
	mat->setShadowMethod(shadow);

	//auto planeGeom = new SphereGeometry(5, 40);
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

	caster = new Mesh(new SphereGeometry(2, 40), color);
	//caster->getGeometries()[0]->calculateNormals();
	caster->getTransform().appendMove(7, 4, 0);

	viewer = new Mesh(new PlaneGeometry(3, 3), make_shared<ShadowCubeTextureMaterial>(shadow->getDebugTexture(), 1));
	//viewer = new Mesh(new PlaneGeometry(3, 3), make_shared<ShadowTextureMaterial>(shadow->getDebugTexture()));
	viewer->getTransform().appendMove(10, 0, 0);

	cube = new Mesh(new CubeGeometry(glm::vec3(20, 20, 10)), make_shared<WireframeMaterial>(1, 1, 1));
	GLfloat* lightDir = light->getDirection();
	//glm::vec3 lightPos = glm::vec3(-lightDir[0], -lightDir[1], -lightDir[2]) * 5.0f;
	glm::vec3 lightPos = plight->getPositionVector();
	cube->getTransform().append(glm::lookAt(lightPos, glm::vec3(0, 0, -lightPos.z), glm::vec3(0, 1, 0)));

	auto shadowLightColor = make_shared<ColorMaterial>(0, 0, 0, 1);
	shadowLightColor->setAmbientColor(1, 1, 0);
	shadowLight = new Mesh(new SphereGeometry(0.3, 20), shadowLightColor);
	shadowLight->getTransform().appendMove(lightPos);

	renderer = new MeshDefaultRenderer();

	// start the main loop
	Engine::start();

	return 0;
}