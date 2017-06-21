#include "glm/gtx/transform.hpp"
#include "pathos/engine.h"
#include "pathos/render/render_forward.h"
#include "pathos/render/envmap.h"
#include "pathos/mesh/mesh.h"
#include "pathos/camera/camera.h"
#include "pathos/light/light.h"
#include "pathos/loader/imageloader.h"
#include "pathos/text/textmesh.h"
#include "pathos/text/font_mgr.h"
#include "pathos/mesh/geometry_primitive.h"
#include <iostream>

using namespace std;
using namespace pathos;

// Camera, scene and renderer
Camera* cam;
Scene scene;
MeshForwardRenderer* renderer;

// 3D objects
TextMesh *label;
Skybox* sky;

// Lights
PointLight *plight;
DirectionalLight *dlight;

void setupModel();
void setupSkybox();

void render() {
	float speedX = 0.05f, speedY = 0.05f;
	float dx = Engine::isDown('a') ? -speedX : Engine::isDown('d') ? speedX : 0.0f;
	float dz = Engine::isDown('w') ? -speedY : Engine::isDown('s') ? speedY : 0.0f;
	float rotY = Engine::isDown('q') ? -0.5f : Engine::isDown('e') ? 0.5f : 0.0f;
	float rotX = Engine::isDown('z') ? -0.5f : Engine::isDown('x') ? 0.5f : 0.0f;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotateY(rotY);
	cam->rotateX(rotX);
	
	// skybox
	renderer->render(&scene, cam);
}

void keyDown(unsigned char ascii, int x, int y) {}

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.width = 800;
	conf.height = 600;
	conf.title = "Test: Text Rendering";
	conf.render = render;
	conf.keyDown = keyDown;
	if (Engine::init(&argc, argv, conf) == false) {
		std::cerr << "Failed to initialize Pathos" << std::endl;
		return 1;
	}

	// camera
	cam = new Camera(new PerspectiveLens(45.0f, 800.0f / 600.0f, 0.1f, 1000.f));
	cam->move(glm::vec3(0, 0, 10));

	// renderer
	renderer = new MeshForwardRenderer;

	// 3d objects
	setupModel();
	setupSkybox();

	// start the main loop
	Engine::start();

	return 0;
}

void setupModel() {
	plight = new PointLight(glm::vec3(5, 30, 5), glm::vec3(1, 1, 1));
	dlight = new DirectionalLight(glm::vec3(0.1, -1, 2), glm::vec3(1, 1, 1));

	FontManager::loadAdditionalGlyphs("hangul", L'��', L'�R');

	label = new TextMesh("hangul");
	label->setText(L"�ѱ� �׽�Ʈ / ��� ������ English Text", 0xff0000);
	label->getTransform().appendScale(2);

	scene.add(plight);
	scene.add(dlight);
	scene.add(label);

	renderer->getShadowMap()->setProjection(glm::ortho(-200.f, 200.f, -200.f, 100.f, -200.f, 500.f));
}

void setupSkybox() {
	const char* cubeImgName[6] = { "../../resources/cubemap1/pos_x.bmp", "../../resources/cubemap1/neg_x.bmp",
		"../../resources/cubemap1/pos_y.bmp", "../../resources/cubemap1/neg_y.bmp",
		"../../resources/cubemap1/pos_z.bmp", "../../resources/cubemap1/neg_z.bmp" };
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	scene.skybox = sky;
}