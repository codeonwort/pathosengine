#include "world2.h"
#include "daeloader.h"
#include "skinned_mesh.h"
#include "player_controller.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/mesh/static_mesh_actor.h"

#include <time.h>

#define DAE_MODEL_ID              2
#define LOAD_SECOND_DAE_MODEL     0


void World2::onInitialize()
{
	playerController = spawnActor<PlayerController>();

	setupScene();
	loadDAE();
}

void World2::onTick(float deltaSeconds)
{
	static float modelYaw = 0.0f;
	model->setActorLocation(vector3(0, 20, 0));
	model->setActorRotation(Rotator(modelYaw += 30.0f * deltaSeconds, 0.0f, 0.0f));
	model->setActorLocation(vector3(0, -20, 0));

	// Dummy boxes
	for (uint32 i = 0; i < (uint32)boxes.size(); ++i)
	{
		Rotator rotator = boxes[i]->getActorRotation();
		rotator.yaw += 30.0f * deltaSeconds;
		boxes[i]->setActorRotation(rotator);
	}

#if DAE_MODEL_ID == 2
	if (daeModel != nullptr) {
		static double time = 0.0;
		time += deltaSeconds;
		if (time > 1.0) time = 0.0;
		daeModel->updateAnimation(0, time);
		daeModel->updateSoftwareSkinning();
	}
#endif

	// Update window title
	{
		char title[256];
		sprintf_s(title, "%s (CPU Time: %.2f ms, GPU Time: %.2f ms)",
			gEngine->getConfig().title, gEngine->getCPUTime(), gEngine->getGPUTime());
		gEngine->getMainWindow()->setTitle(title);
	}
}

void World2::setupScene()
{
	srand(static_cast<unsigned int>(time(NULL)));

	dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setLightParameters(vector3(0.0f, 0.0f, -1.0f), vector3(1.0f));

	pointLight0 = spawnActor<PointLightActor>();
	pointLight0->setActorLocation(vector3(0.0f, 0.0f, 0.0f));
	pointLight0->setLightParameters(vector3(1.0f));

	//---------------------------------------------------------------------------------------
	// create materials
	//---------------------------------------------------------------------------------------
	std::array<const char*, 6> cubeImgName = {
		"cubemap1/pos_x.jpg", "cubemap1/neg_x.jpg",
		"cubemap1/pos_y.jpg", "cubemap1/neg_y.jpg",
		"cubemap1/pos_z.jpg", "cubemap1/neg_z.jpg"
	};
	std::array<FIBITMAP*, 6> cubeImg;
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

	//---------------------------------------------------------------------------------------
	// create meshes
	//---------------------------------------------------------------------------------------

	for (int32_t i = 0; i < 8; ++i) {
		for (int32_t j = 0; j < 4; ++j) {
			StaticMeshActor* cube = spawnActor<StaticMeshActor>();
			cube->setStaticMesh(new Mesh(geom_cube, material_color));

			vector3 p0(-150.0f, 150.0f, -50.0f);
			float yaw = (float)(rand() % 180);
			float pitch = (float)(rand() % 90);
			float roll = (float)(rand() % 180);
			cube->setActorRotation(Rotator(yaw, pitch, roll));
			cube->setActorLocation(p0 + vector3(i * 45.0f, -j * 45.0f, 0.0f));
			cube->setActorScale(3.0f);

			boxes.push_back(cube);
		}
	}

	model = spawnActor<StaticMeshActor>();
	model->setStaticMesh(new Mesh(geom_sphere_big, material_texture));
	model->setActorLocation(vector3(-40.0f, 0.0f, 0.0f));

	model2 = spawnActor<StaticMeshActor>();
	model2->setStaticMesh(new Mesh(geom_sphere, material_color));
	model2->setActorScale(30.0f);
	model2->setActorLocation(vector3(0.0f, 50.0f, -400.0f));

	scene.sky = new Skybox(cubeTexture);
	scene.godRaySource = model2->getStaticMeshComponent();
}

void World2::loadDAE()
{
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

		StaticMeshActor* daeActor = spawnActor<StaticMeshActor>();
		daeActor->setStaticMesh(daeModel);

		daeActor->setActorScale(10.0f);
#if DAE_MODEL_ID == 2
		daeActor->setActorScale(10.0f);
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

