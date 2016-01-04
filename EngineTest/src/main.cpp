#include <iostream>
#include <pathos/engine.h>
#include <pathos/mesh/mesh.h>
#include <pathos/mesh/render.h>
#include <pathos/camera/camera.h>
#include <pathos/light/light.h>
#include <pathos/loader/imageloader.h>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace pathos;

Camera* cam;
Mesh *plane, *cube;
Mesh *caster, *viewer;
MeshDefaultRenderer* renderer;

void render() {
	float dx = Engine::isDown('a') ? -0.1 : Engine::isDown('d') ? 0.1 : 0;
	float dz = Engine::isDown('w') ? -0.1 : Engine::isDown('s') ? 0.1 : 0;
	float dr = Engine::isDown('q') ? 0.5 : Engine::isDown('e') ? -0.5 : 0;
	float dr2 = Engine::isDown('z') ? 0.5 : Engine::isDown('x') ? -0.5 : 0;
	cam->move(glm::vec3(dx, 0, dz));
	cam->rotate(dr, glm::vec3(0, 1, 0));
	cam->rotate(dr2, glm::vec3(1, 0, 0));

	renderer->ready();
	renderer->render(caster, cam);
	renderer->render(plane, cam);
	renderer->render(viewer, cam);
	renderer->render(cube, cam);
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

	auto light = new DirectionalLight(glm::vec3(0, -1, -0.1));
	auto shadow = new ShadowMap(light, cam);

	auto plight = new PointLight(glm::vec3(0, 5, 5), glm::vec3(0, 0, 1));
	auto plight2 = new PointLight(glm::vec3(5, 0, 5), glm::vec3(1, 0, 0));
	auto plight3 = new PointLight(glm::vec3(-5, -2, 3), glm::vec3(0, 1, 0));

	auto color = make_shared<ColorMaterial>(1.0, 1.0, 1.0, 1);
	//color->setAmbientColor(0, 0, 0);
	color->addLight(plight);
	color->addLight(plight2);
	color->addLight(plight3);
	//color->setShadowMethod(shadow);

	auto planeColor = make_shared<ColorMaterial>(0.0, 1.0, 1.0, 1);
	planeColor->setAmbientColor(0, 0, 1);
	planeColor->addLight(light);
	planeColor->setShadowMethod(shadow);

	GLuint tex = loadTexture(loadImage("../resources/image2.jpg"));
	auto mat = make_shared<TextureMaterial>(tex);
	mat->addLight(light);
	mat->setShadowMethod(shadow);

	auto planeGeom = new PlaneGeometry(25, 25, 30, 30);
	auto test = planeGeom->getPos();
	int k = 2;
	for (int i = 0; i <= 30; i++){
		for (int j = 0; j <= 30; j++){
			test[k] = ((i*j) % 7) / 5.0;
			k += 3;
		}
	}
	planeGeom->uploadPosition();
	planeGeom->calculateNormals();

	plane = new Mesh(planeGeom, planeColor);
	plane->getTransform().appendRotation(glm::radians(-85.0), glm::vec3(1, 0, 0));
	plane->getTransform().appendMove(0, -0.5, -4);

	caster = new Mesh(new SphereGeometry(2, 40), color);
	//caster->getGeometries()[0]->calculateNormals();
	caster->getTransform().appendMove(0, 2, -4);

	viewer = new Mesh(new PlaneGeometry(3, 3), make_shared<ShadowTextureMaterial>(shadow->getDebugTexture()));
	viewer->getTransform().appendMove(10, 0, 0);

	cube = new Mesh(new CubeGeometry(glm::vec3(20, 20, 10)), make_shared<WireframeMaterial>(1, 1, 1));
	GLfloat* lightDir = light->getDirection();
	glm::vec3 lightPos = glm::vec3(-lightDir[0], -lightDir[1], -lightDir[2]) * 5.0f;
	cube->getTransform().append(glm::lookAt(lightPos, glm::vec3(0, 0, -lightPos.z), glm::vec3(0, 1, 0)));

	renderer = new MeshDefaultRenderer();

	// start the main loop
	Engine::start();

	return 0;
}