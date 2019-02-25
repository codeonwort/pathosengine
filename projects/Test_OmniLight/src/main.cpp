#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
using namespace pathos;

// Rendering configurations
const int WINDOW_WIDTH  = 1920;
const int WINDOW_HEIGHT = 1080;
const float FOV         = 90.0f;
const char* TITLE       = "Test: Omnidirectional Light";

// world
Camera* cam;
Scene scene;
	Mesh *cube, *viewer, *shadowLight; // shadow debugger
	Mesh *ball, *lamp; // shadow casters
	Mesh *plane_posX, *plane_negX, *plane_posY, *plane_negY, *plane_posZ, *plane_negZ; // shadow receivers
	Skybox* sky;

void setupScene();

void tick() {
	if (gConsole->isVisible() == false) {
		float speedX = 0.5f, speedY = 0.5f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -1.5f : gEngine->isDown('e') ? 1.5f : 0.0f;
		float rotX = gEngine->isDown('z') ? -2.0f : gEngine->isDown('x') ? 2.0f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}
	lamp->getTransform().appendRotation(0.001f, glm::vec3(1.0f, 0.5f, 0.f));
}

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = TITLE;
	conf.rendererType = ERendererType::Forward;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	const float aspectRatio = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, aspectRatio, 1.0f, 10000.0f));
	cam->lookAt(glm::vec3(0, 0, 30), glm::vec3(5, 0, 0), glm::vec3(0, 1, 0));

	setupScene();

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	return 0;
}

void setupScene() {
	auto pointLight = new PointLight(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1));
	//scene.add(new DirectionalLight(glm::vec3(0, -1, -0.1)));
	scene.add(pointLight);

	OBJLoader obj2("models/lightbulb.obj", "models/");
	lamp = obj2.craftMeshFromAllShapes();
	lamp->getTransform().appendScale(4.0f);
	lamp->doubleSided = true;

	const char* cubeImgName[6] = {
		"cubemap1/pos_x.jpg", "cubemap1/neg_x.jpg",
		"cubemap1/pos_y.jpg", "cubemap1/neg_y.jpg",
		"cubemap1/pos_z.jpg", "cubemap1/neg_z.jpg"
	};
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);
	scene.sky = sky;

	// bump texture for planes
	GLuint tex = loadTexture(loadImage("154.jpg"), true, true);
	GLuint tex_norm = loadTexture(loadImage("154_norm.jpg"), true, false);
	auto mat = new BumpTextureMaterial(tex, tex_norm);

	auto planeGeom = new PlaneGeometry(30, 30);
	planeGeom->calculateTangentBasis();

	plane_posX = new Mesh(planeGeom, mat);
	plane_posX->getTransform().appendRotation(glm::radians(-90.0f), glm::vec3(0, 1, 0));
	plane_posX->getTransform().appendMove(15, 0, 0);

	plane_negX = new Mesh(planeGeom, mat);
	plane_negX->getTransform().appendRotation(glm::radians(90.0f), glm::vec3(0, 1, 0));
	plane_negX->getTransform().appendMove(-15, 0, 0);

	plane_posY = new Mesh(planeGeom, mat);
	plane_posY->getTransform().appendRotation(glm::radians(90.0f), glm::vec3(1, 0, 0));
	plane_posY->getTransform().appendMove(0, 15, 0);

	plane_negY = new Mesh(planeGeom, mat);
	plane_negY->getTransform().appendRotation(glm::radians(-90.0f), glm::vec3(1, 0, 0));
	plane_negY->getTransform().appendMove(0, -15, 0);

	plane_posZ = new Mesh(planeGeom, mat);
	plane_posZ->getTransform().appendRotation(glm::radians(180.0f), glm::vec3(0, 1, 0));
	plane_posZ->getTransform().appendMove(0, 0, 15);

	plane_negZ = new Mesh(planeGeom, mat);
	plane_negZ->getTransform().appendMove(0, 0, -15);

	auto color = new ColorMaterial;

	ball = new Mesh(new SphereGeometry(2, 40), color);
	ball->getTransform().appendMove(7, 4, 0);

	cube = new Mesh(new CubeGeometry(glm::vec3(20, 20, 10)), new WireframeMaterial(1, 1, 1));
	glm::vec3 lightPos = pointLight->getPosition();
	cube->getTransform().append(glm::lookAt(lightPos, glm::vec3(0, 0, -lightPos.z), glm::vec3(0, 1, 0)));

	auto shadowLightColor = new ColorMaterial;
	shadowLightColor->setAmbient(1, 1, 0);
	shadowLightColor->setDiffuse(0, 0, 0);
	shadowLight = new Mesh(new SphereGeometry(0.3f, 20), shadowLightColor);
	shadowLight->getTransform().appendMove(lightPos);

	// scene configuration
	scene.sky = sky;
	scene.add(ball);
	scene.add(lamp);
	scene.add(shadowLight);
	scene.add({ plane_posX, plane_negX, plane_posY, plane_negY, plane_posZ, plane_negZ });
}
