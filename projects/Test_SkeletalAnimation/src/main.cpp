#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"

#include "daeloader.h"
#include "skinned_mesh.h"
#include <time.h>

using namespace std;
using namespace pathos;

/* ------------------------------------------------------------------------

								CONFIGURATION

------------------------------------------------------------------------- */
#define DAE_MODEL_ID              2
#define LOAD_SECOND_DAE_MODEL     0

constexpr int WINDOW_WIDTH      = 1920;
constexpr int WINDOW_HEIGHT     = 1080;
constexpr char* WINDOW_TITLE    = "Test: Skeletal Animation";
constexpr float FOV             = 90.0f;
const glm::vec3 CAMERA_POSITION = glm::vec3(0.0f, 0.0f, 100.0f);
constexpr float CAMERA_Z_NEAR   = 1.0f;
constexpr float CAMERA_Z_FAR    = 10000.0f;

/* ------------------------------------------------------------------------

								PROGRAM CODE

------------------------------------------------------------------------- */

Camera* cam;
Scene scene;
	DirectionalLight *dlight;
	PointLight *plight;
	Mesh *model, *model2;
	SkinnedMesh *daeModel;
	SkinnedMesh *daeModel2;

void loadDAE();
void setupScene();
void tick();

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = WINDOW_TITLE;
	conf.rendererType = ERendererType::Deferred;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	const float ar = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, ar, CAMERA_Z_NEAR, CAMERA_Z_FAR));
	cam->move(CAMERA_POSITION);

	loadDAE();
	setupScene();

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	return 0;
}

void loadDAE() {
	bool invertWinding = false;
#if DAE_MODEL_ID == 0
	const std::string dir = "models/LOL_ashe/";
	const std::string model = dir + "Ashe.dae";
#elif DAE_MODEL_ID == 1
	const std::string dir = "models/LOL_project_ashe/";
	const std::string model = dir + "Project_Ashe.dae";
	invertWinding = true;
#else
	const std::string dir = "models/animtest/";
	const std::string model = dir + "animtest.dae";
#endif

	DAELoader loader(model.c_str(), dir.c_str(), aiProcessPreset_TargetRealtime_MaxQuality, invertWinding);
	if (loader.getMesh()) {
		daeModel = dynamic_cast<SkinnedMesh*>(loader.getMesh());
		daeModel->getTransform().appendScale(10.0f);
#if DAE_MODEL_ID == 2
		daeModel->getTransform().appendScale(0.3f);
		daeModel->getTransform().appendRotation(glm::radians(90.0f), glm::vec3(0, 1, 0));
#endif
		daeModel->getTransform().appendMove(0, 0, 50);
		scene.add(daeModel);
	} else {
		LOG(LogError, "Failed to load model: %s", model.c_str());
	}

#if LOAD_SECOND_DAE_MODEL
	const std::string dir2 = "models/Sonic/";
	const std::string model2 = dir2 + "Sonic.dae";
	DAELoader loader2(model2.c_str(), dir2.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
	daeModel2 = dynamic_cast<SkinnedMesh*>(loader2.getMesh());
	daeModel2->getTransform().appendScale(10.0f);
	daeModel2->getTransform().appendMove(20.0f, 0.0f, 50.0f);
	scene.add(daeModel2);
#endif
}

void setupScene() {
	plight = new PointLight(glm::vec3(0, 0, 0), glm::vec3(1, 0, 1));
	dlight = new DirectionalLight(glm::vec3(0, 0, -1), glm::vec3(1.0f));
	scene.add(plight);
	scene.add(dlight);

	srand(static_cast<unsigned int>(time(NULL)));

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
	auto geom_sphere = new SphereGeometry(5.0f, 30);
	auto geom_plane = new PlaneGeometry(10.f, 10.f);
	auto geom_cube = new CubeGeometry(glm::vec3(5.0f));

	geom_sphere->calculateTangentBasis();
	geom_sphere_big->calculateTangentBasis();
	geom_plane->calculateTangentBasis();
	geom_cube->calculateTangentBasis();

	//---------------------------------------------------------------------------------------
	// create meshes
	//---------------------------------------------------------------------------------------

	for (int32_t i = 0; i < 4; ++i) {
		for (int32_t j = 0; j < 4; ++j) {
			Mesh* cube = new Mesh(geom_cube, material_color);
			glm::vec3 p0(-50.0f, 50.0f, -50.0f);
			float e = (rand() % 256) / 255.0f;
			float x = (rand() % 256) / 255.0f;
			float y = (rand() % 256) / 255.0f;
			float z = (rand() % 256) / 255.0f;
			cube->getTransform().appendRotation(e * 60.0f, glm::vec3(x, y, z));
			cube->getTransform().appendMove(p0 + glm::vec3(i * 15.0f, -j * 15.0f, 0.0f));
			scene.add(cube);
		}
	}

	model = new Mesh(geom_sphere_big, material_texture);
	model->getTransform().appendMove(-40, 0, 0);

	model2 = new Mesh(geom_sphere, material_color);
	model2->getTransform().appendScale(10.0f);
	model2->getTransform().appendMove(0, 50, -200);

	scene.add(model);
	scene.add(model2);
	scene.sky = new Skybox(cubeTexture);
	scene.godRaySource = model2;
}

void tick() {
	if (gConsole->isVisible() == false) {
		float speedX = 1.0f, speedY = 1.0f;
		float dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -0.5f : gEngine->isDown('e') ? 0.5f : 0.0f;
		float rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}

	model->getTransform().appendMove(0, 20, 0);
	model->getTransform().appendRotation(0.01f, glm::vec3(0, 0.5, 1));
	model->getTransform().appendMove(0, -20, 0);

#if DAE_MODEL_ID == 2
	static double time = 0.0;
	time += 0.01;
	if (time > 1.0) time = 0.0;
	daeModel->updateAnimation(0, time);
	daeModel->updateSoftwareSkinning();
	daeModel->getTransform().prependRotation(0.003f, glm::vec3(0, 1, 0));
#endif
}
