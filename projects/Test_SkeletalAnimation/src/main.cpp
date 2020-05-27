#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/input/input_manager.h"
#include "pathos/gui/gui_window.h"
#include "pathos/light/light_all.h"
#include "pathos/mesh/static_mesh_actor.h"

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

constexpr int32     WINDOW_WIDTH    = 1920;
constexpr int32     WINDOW_HEIGHT   = 1080;
constexpr char*     WINDOW_TITLE    = "Test: Skeletal Animation";
constexpr float     FOVY            = 60.0f;
const     vector3   CAMERA_POSITION = vector3(0.0f, 0.0f, 300.0f);
constexpr float     CAMERA_Z_NEAR   = 1.0f;
constexpr float     CAMERA_Z_FAR    = 10000.0f;

/* ------------------------------------------------------------------------

								PROGRAM CODE

------------------------------------------------------------------------- */

Camera* cam;
Scene scene;
	StaticMeshActor *model, *model2;
	SkinnedMesh *daeModel;
	SkinnedMesh *daeModel2;

void setupInput();
void loadDAE();
void setupScene();
void setupSceneWithActor(Scene* scene);
void tick(float deltaSeconds);

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = WINDOW_TITLE;
	conf.rendererType = ERendererType::Deferred;
	conf.tick         = tick;
	Engine::init(argc, argv, conf);

	setupInput();

	const float ar = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOVY, ar, CAMERA_Z_NEAR, CAMERA_Z_FAR));
#if 0 // #todo-camera: Eye have to see +z when yaw=0, but it looks at -z !!!
	cam->moveToPosition(CAMERA_POSITION);
	cam->setYaw(0.0f);
	cam->setPitch(0.0f);
#else
	cam->lookAt(CAMERA_POSITION, CAMERA_POSITION + vector3(0.0f, 0.0f, -1.0f), vector3(0.0f, 1.0f, 0.0f));
#endif

	loadDAE();
	setupScene();
	setupSceneWithActor(&scene);

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	return 0;
}

void setupInput()
{
	AxisBinding moveForward;
	moveForward.addInput(InputConstants::KEYBOARD_W, 1.0f);
	moveForward.addInput(InputConstants::KEYBOARD_S, -1.0f);

	AxisBinding moveRight;
	moveRight.addInput(InputConstants::KEYBOARD_D, 1.0f);
	moveRight.addInput(InputConstants::KEYBOARD_A, -1.0f);

	AxisBinding rotateYaw;
	rotateYaw.addInput(InputConstants::KEYBOARD_Q, -1.0f);
	rotateYaw.addInput(InputConstants::KEYBOARD_E, 1.0f);

	AxisBinding rotatePitch;
	rotatePitch.addInput(InputConstants::KEYBOARD_Z, -1.0f);
	rotatePitch.addInput(InputConstants::KEYBOARD_X, 1.0f);

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindAxis("moveForward", moveForward);
	inputManager->bindAxis("moveRight", moveRight);
	inputManager->bindAxis("rotateYaw", rotateYaw);
	inputManager->bindAxis("rotatePitch", rotatePitch);
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

		StaticMeshActor* daeActor = scene.spawnActor<StaticMeshActor>();
		daeActor->setStaticMesh(daeModel);
		
		daeActor->setActorScale(10.0f);
#if DAE_MODEL_ID == 2
		daeActor->setActorScale(5.0f);
		daeActor->setActorRotation(Rotator(90.0f, 0.0f, 0.0f));
#endif
		daeActor->setActorLocation(vector3(0.0f, 0.0f, 0.0f));
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

void setupSceneWithActor(Scene* scene) {
	DirectionalLightActor* dirLight = scene->spawnActor<DirectionalLightActor>();
	dirLight->setLightParameters(vector3(0.0f, 0.0f, -1.0f), vector3(1.0f));

	PointLightActor* pointLight0 = scene->spawnActor<PointLightActor>();
	pointLight0->setActorLocation(vector3(0.0f, 0.0f, 0.0f));
	pointLight0->setLightParameters(vector3(1.0f));
}
void setupScene() {
	srand(static_cast<unsigned int>(time(NULL)));

	//---------------------------------------------------------------------------------------
	// create materials
	//---------------------------------------------------------------------------------------
	std::array<const char*,6> cubeImgName = {
		"cubemap1/pos_x.jpg", "cubemap1/neg_x.jpg",
		"cubemap1/pos_y.jpg", "cubemap1/neg_y.jpg",
		"cubemap1/pos_z.jpg", "cubemap1/neg_z.jpg"
	};
	std::array<FIBITMAP*,6> cubeImg;
	pathos::loadCubemapImages(cubeImgName, ECubemapImagePreference::HLSL, cubeImg);
	GLuint cubeTexture = createCubemapTextureFromBitmap(cubeImg.data());

	GLuint tex = createTextureFromBitmap(loadImage("154.jpg"), true, true);
	GLuint tex_norm = createTextureFromBitmap(loadImage("154_norm.jpg"), true, false);

	auto material_texture = new TextureMaterial(tex);
	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(1.0f, 1.0f, 1.0f);
		color->setMetallic(0.6f);
		color->setRoughness(0.5f);
	}
	auto material_cubemap = new CubeEnvMapMaterial(cubeTexture);
	auto material_wireframe = new WireframeMaterial(0.0f, 1.0f, 1.0f, 0.3f);

	//---------------------------------------------------------------------------------------
	// create geometries
	//---------------------------------------------------------------------------------------
	
	auto geom_sphere_big = new SphereGeometry(15.0f, 30);
	auto geom_sphere = new SphereGeometry(5.0f, 30);
	auto geom_plane = new PlaneGeometry(10.f, 10.f);
	auto geom_cube = new CubeGeometry(vector3(5.0f));

	geom_sphere->calculateTangentBasis();
	geom_sphere_big->calculateTangentBasis();
	geom_plane->calculateTangentBasis();
	geom_cube->calculateTangentBasis();

	//---------------------------------------------------------------------------------------
	// create meshes
	//---------------------------------------------------------------------------------------

	for (int32_t i = 0; i < 8; ++i) {
		for (int32_t j = 0; j < 4; ++j) {
			StaticMeshActor* cube = scene.spawnActor<StaticMeshActor>();
			cube->setStaticMesh(new Mesh(geom_cube, material_color));
			vector3 p0(-50.0f, 50.0f, -50.0f);
			float yaw = (float)(rand() % 180);
			float pitch = (float)(rand() % 90);
			float roll = (float)(rand() % 180);
			cube->setActorRotation(Rotator(yaw, pitch, roll));
			cube->setActorLocation(p0 + vector3(i * 15.0f, -j * 15.0f, 0.0f));
		}
	}

	model = scene.spawnActor<StaticMeshActor>();
	model->setStaticMesh(new Mesh(geom_sphere_big, material_texture));
	model->setActorLocation(vector3(-40.0f, 0.0f, 0.0f));

	model2 = scene.spawnActor<StaticMeshActor>();
	model2->setStaticMesh(new Mesh(geom_sphere, material_color));
	model2->setActorScale(10.0f);
	model2->setActorLocation(vector3(0.0f, 50.0f, -200.0f));

	scene.sky = new Skybox(cubeTexture);
	scene.godRaySource = model2->getStaticMeshComponent();
}

void tick(float deltaSeconds) {
	{
		InputManager* input = gEngine->getInputSystem()->getDefaultInputManager();

		// movement per seconds
		const float speedRight = 400.0f * deltaSeconds;
		const float speedForward = 200.0f * deltaSeconds;
		const float rotateY = 120.0f * deltaSeconds;
		const float rotateX = 120.0f * deltaSeconds;

		float moveRight = input->getAxis("moveRight") * speedRight;
		float moveForward = input->getAxis("moveForward") * speedForward;
		float rotY = input->getAxis("rotateYaw") * rotateY;
		float rotX = input->getAxis("rotatePitch") * rotateX;

		cam->move(vector3(moveForward, moveRight, 0.0f));
		cam->rotateYaw(rotY);
		cam->rotatePitch(rotX);
	}

	static float modelYaw = 0.0f;
	model->setActorLocation(vector3(0, 20, 0));
	model->setActorRotation(Rotator(modelYaw += 30.0f * deltaSeconds, 0.0f, 0.0f));
	model->setActorLocation(vector3(0, -20, 0));

#if DAE_MODEL_ID == 2
	static double time = 0.0;
	time += deltaSeconds;
	if (time > 1.0) time = 0.0;
	daeModel->updateAnimation(0, time);
	daeModel->updateSoftwareSkinning();
#endif

	{
		char title[256];
		sprintf_s(title, "%s (CPU Time: %.2f ms, GPU Time: %.2f ms)", WINDOW_TITLE, gEngine->getCPUTime(), gEngine->getGPUTime());
		gEngine->getMainWindow()->setTitle(title);
	}
}
