#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
using namespace pathos;

constexpr int   WINDOW_WIDTH  = 1920;
constexpr int   WINDOW_HEIGHT = 1080;
constexpr char* WINDOW_TITLE  = "Test: Refactor Forward Renderer";
constexpr float FOV           = 90.0f;

Camera* cam;
Scene scene;
	Mesh *model, *model2, *model3, *model4;
	Mesh *model_shadowdebug;

DirectionalLight *sunLight;
PointLight *pointLight;

void setupScene();
void tick();

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = WINDOW_TITLE;
	conf.rendererType = ERendererType::Forward;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	cam = new Camera(new PerspectiveLens(FOV / 2.0f, (float)conf.windowWidth / conf.windowHeight, 1.0f, 1000.f));
	cam->move(glm::vec3(0.0f, 0.0f, 50.0f));

	setupScene();

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	return 0;
}

void setupScene() {
	pointLight = new PointLight(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
	sunLight = new DirectionalLight(glm::vec3(1, 0, 0), glm::vec3(1, 1, 1));
	scene.add(pointLight);
	scene.add(sunLight);

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

	GLuint tex = loadTexture(loadImage("154.jpg"), true, true);
	GLuint tex_norm = loadTexture(loadImage("154_norm.jpg"), true, false);

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

	model = new Mesh(geom_sphere_big, material_bump);
	model->getTransform().appendMove(30, 15, 0);

	model2 = new Mesh(geom_sphere, material_color);
	model2->getTransform().appendRotation(glm::radians(-10.f), glm::vec3(0, 1, 0));
	model2->getTransform().appendMove(10, 0, 0);

	model3 = new Mesh(geom_cube, material_wireframe);
	model3->getTransform().appendMove(30, 30, 0);

	model4 = new Mesh(geom_sphere_big, material_cubemap);
	model4->getTransform().appendMove(30, -15, 0);

	scene.sky = new Skybox(cubeTexture);
	scene.add({ model, model2, model3, model4 });
}

void tick() {
	if (gConsole->isVisible() == false) {
		float speedX = 0.4f;
		float speedY = 0.4f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -0.5f : gEngine->isDown('e') ? 0.5f : 0.0f;
		float rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}

	model2->getTransform().appendMove(30, 0, 0);
	model2->getTransform().appendRotation(0.01f, glm::vec3(0, 1, 0));
	model2->getTransform().appendMove(-30, 0, 0);
}
