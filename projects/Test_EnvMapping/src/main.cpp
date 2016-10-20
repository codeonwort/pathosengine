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
shared_ptr<MeshMaterial> envMapMaterial = nullptr;
Mesh *teapot;
TextMesh *label;
Skybox* sky;

// Lights and shadow
PointLight *plight, *plight2;
OmnidirectionalShadow* shadow;

void setupModel();
void setupSkybox();

void render() {
	float speedX = 0.1f, speedY = 0.1f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
	float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);

	renderer->ready();

	// skybox
	renderer->render(sky, cam);

	// various models
	renderer->render(teapot, cam);
	renderer->render(label, cam);
}

void keyDown(unsigned char ascii, int x, int y) {}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.width = 800;
	conf.height = 600;
	conf.title = "Test: Envrionmental Mapping";
	conf.render = render;
	conf.keyDown = keyDown;
	Engine::init(&argc, argv, conf);

	// camera
	cam = new Camera(new PerspectiveLens(45.0f, 800.0f / 600.0f, 0.1f, 1000.f));
	cam->move(glm::vec3(0, 0, 20));

	// renderer
	renderer = new MeshDefaultRenderer();

	// 3d objects
	setupSkybox();
	setupModel();

	// start the main loop
	Engine::start();

	return 0;
}

void setupModel() {
	plight = new PointLight(glm::vec3(5, 30, 5), glm::vec3(1, 1, 1));
	plight2 = new PointLight(glm::vec3(-15, 30, 5), glm::vec3(0, 0, 1));
	shadow = new OmnidirectionalShadow(plight, cam);

	label = new TextMesh("default");
	label->setText("Working on Envrionmental mapping", 0xffff00);
	label->getTransform().appendScale(10, 10, 10);
	label->setDoubleSided(true);

	/*OBJLoader teapotLoader("../../resources/models/teapot/teapot.obj", "../../resources/models/teapot/");
	teapot = new Mesh();
	for (int i = 0; i < teapotLoader.getGeometries().size(); i++){
		if (envMapMaterial == nullptr){
			cerr << "Env map material is not initialized!" << endl;
		}
		teapot->add(teapotLoader.getGeometries()[i], envMapMaterial);
	}
	teapot->getTransform().appendScale(.2, .2, .2);
	teapot->getTransform().appendMove(0, -40, -50);*/
	teapot = new Mesh(new SphereGeometry(5, 20), envMapMaterial);
}

void setupSkybox() {
	const char* cubeImgName[6] = { "../../resources/cubemap1/pos_x.bmp", "../../resources/cubemap1/neg_x.bmp",
		"../../resources/cubemap1/pos_y.bmp", "../../resources/cubemap1/neg_y.bmp",
		"../../resources/cubemap1/pos_z.bmp", "../../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	envMapMaterial = make_shared<CubeEnvMapMaterial>(cubeTex);
}