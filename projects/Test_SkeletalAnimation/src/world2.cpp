#include "world2.h"
#include "daeloader.h"
#include "skinned_mesh.h"
#include "player_controller.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/skybox_actor.h"
#include "pathos/text/text_actor.h"
#include "pathos/loader/asset_streamer.h"

#include <time.h>

#define FOV_Y                     60.0f
#define SUN_DIRECTION             glm::normalize(vector3(0.0f, -1.0f, -1.0f))
#define SUN_ILLUMINANCE           vector3(2.0f)
#define CAMERA_POSITION           vector3(0.0f, 2.0f, 6.0f)
#define CAMERA_LOOK_AT            vector3(0.0f, 2.0f, 4.0f)

#define DIR_MY_ANIMTEST           "resources/models/animtest/"
#define FILE_MY_ANIMTEST          "resources/models/animtest/animtest.dae"
#define DIR_RIGGED_FIGURE         "resources_external/KhronosGroup/RiggedFigure/"
#define FILE_RIGGED_FIGURE        "resources_external/KhronosGroup/RiggedFigure/RiggedFigure.dae"
#define FILE_LPS_HEAD             "resources_external/LPSHead/head.obj"
#define DIR_LPS_HEAD              "resources_external/LPSHead/"
#define DOWNLOAD_ALERT_MSG1       L"If you can't see 2 animated models, run Setup.ps1"
#define DOWNLOAD_ALERT_MSG2       L"움직이는 모델 2개가 표시되지 않으면 Setup.ps1을 실행해주세요"


void World2::onInitialize()
{
	AssetReferenceWavefrontOBJ assetRef(FILE_LPS_HEAD, DIR_LPS_HEAD);
	gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRef, this, &World2::onLoadWavefrontOBJ, 0);

	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));

	playerController = spawnActor<PlayerController>();

	setupScene();
	loadDAE();
}

void World2::onTick(float deltaSeconds)
{
	static double animTime = 0.0;
	animTime += deltaSeconds;

	if (daeModel_my != nullptr) {
		double duration = daeModel_my->getAnimationInfo(0)->getLength();
		double time = ::fmod(animTime, duration * 2);
		if (time > duration) time = duration * 2 - time;
		daeModel_my->updateAnimation(0, time);
		daeModel_my->updateSoftwareSkinning();
	}
	if (daeModel_riggedFigure != nullptr)
	{
		double duration = daeModel_riggedFigure->getAnimationInfo(0)->getLength();
		double time = ::fmod(animTime, duration * 2);
		if (time > duration) time = duration * 2 - time;
		daeModel_riggedFigure->updateAnimation(0, time);
		daeModel_riggedFigure->updateSoftwareSkinning();
	}
}

void World2::setupScene()
{
	srand(static_cast<unsigned int>(time(NULL)));

	sunLight = spawnActor<DirectionalLightActor>();
	sunLight->setDirection(SUN_DIRECTION);
	sunLight->setIlluminance(SUN_ILLUMINANCE);

	pointLight0 = spawnActor<PointLightActor>();
	pointLight0->setActorLocation(vector3(0.0f, 2.0f, 2.0f));
	pointLight0->setIntensity(100.0f * vector3(1.0f, 5.0f, 1.0f));
	pointLight0->setAttenuationRadius(10.0f);
	pointLight0->setSourceRadius(0.2f);

	//---------------------------------------------------------------------------------------
	// Materials

	std::array<const char*, 6> cubeImageNames = {
		"skybox/cubemap1/pos_x.jpg", "skybox/cubemap1/neg_x.jpg",
		"skybox/cubemap1/pos_y.jpg", "skybox/cubemap1/neg_y.jpg",
		"skybox/cubemap1/pos_z.jpg", "skybox/cubemap1/neg_z.jpg"
	};
	std::array<BitmapBlob*, 6> cubeImageBlobs;
	pathos::loadCubemapImages(cubeImageNames, ECubemapImagePreference::HLSL, cubeImageBlobs);
	GLuint cubeTexture = pathos::createCubemapTextureFromBitmap(cubeImageBlobs.data());

	Material* material_color = Material::createMaterialInstance("solid_color");
	material_color->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	material_color->setConstantParameter("metallic", 0.0f);
	material_color->setConstantParameter("roughness", 0.5f);
	material_color->setConstantParameter("emissive", vector3(0.0f));

	Material* material_ground = Material::createMaterialInstance("solid_color");
	material_ground->setConstantParameter("albedo", vector3(0.5f, 0.5f, 0.5f));
	material_ground->setConstantParameter("metallic", 0.0f);
	material_ground->setConstantParameter("roughness", 1.0f);
	material_ground->setConstantParameter("emissive", vector3(0.0f));

	//---------------------------------------------------------------------------------------
	// Geometries

	auto geom_sphere = new SphereGeometry(5.0f, 30);
	auto geom_plane = new PlaneGeometry(100.0f, 100.0f);

	//---------------------------------------------------------------------------------------
	// Actors

	ground = spawnActor<StaticMeshActor>();
	ground->setStaticMesh(new Mesh(geom_plane, material_ground));
	ground->setActorLocation(0.0f, 0.0f, 0.0f);
	ground->setActorRotation(Rotator(0.0f, -90.0f, 0.0f));
	ground->getStaticMesh()->doubleSided = true;

	godRaySourceMesh = spawnActor<StaticMeshActor>();
	godRaySourceMesh->setStaticMesh(new Mesh(geom_sphere, material_color));
	godRaySourceMesh->setActorLocation(vector3(0.0f, 100.0f, -500.0f));

	alertText1 = spawnActor<TextMeshActor>();
	alertText1->setFont("hangul");
	alertText1->setText(DOWNLOAD_ALERT_MSG1);
	alertText1->setActorLocation(-5.0f, 4.0f, 0.0f);
	alertText1->setColor(0.3f, 0.9f, 0.3f);
	alertText1->setActorScale(8.0f);

	alertText2 = spawnActor<TextMeshActor>();
	alertText2->setFont("hangul");
	alertText2->setText(DOWNLOAD_ALERT_MSG2);
	alertText2->setActorLocation(-5.0f, 3.0f, 0.0f);
	alertText2->setColor(0.9f, 0.1f, 0.9f);
	alertText2->setActorScale(8.0f);

	SkyboxActor* sky = spawnActor<SkyboxActor>();
	sky->initialize(cubeTexture);

	scene.sky = sky;
	scene.godRaySource = godRaySourceMesh->getStaticMeshComponent();
}

void World2::loadDAE()
{
	auto debugPrintDAE = [](SkinnedMesh* skinnedMesh) {
		LOG(LogInfo, "Num animations: %u", skinnedMesh->numAnimations());
		for (size_t i = 0; i < skinnedMesh->numAnimations(); ++i)
		{
			SkeletalAnimation* anim = skinnedMesh->getAnimationInfo(i);
			LOG(LogInfo, "    length=%lf", anim->getLength());
			LOG(LogInfo, "    name=%s", anim->getName().c_str());
		}
	};

	DAELoader loader1(FILE_MY_ANIMTEST, DIR_MY_ANIMTEST);
	if (loader1.getMesh()) {
		daeModel_my = dynamic_cast<SkinnedMesh*>(loader1.getMesh());
		debugPrintDAE(daeModel_my);

		StaticMeshActor* daeActor = spawnActor<StaticMeshActor>();
		daeActor->setStaticMesh(daeModel_my);

		daeActor->setActorLocation(vector3(0.0f, 0.0f, 0.0f));
		daeActor->setActorRotation(Rotator(90.0f, 0.0f, 0.0f));
		daeActor->setActorScale(0.2f);
	} else {
		LOG(LogError, "Failed to load model: %s", FILE_MY_ANIMTEST);
	}

	DAELoader loader2(FILE_RIGGED_FIGURE, DIR_RIGGED_FIGURE);
	if (loader2.getMesh()) {
		daeModel_riggedFigure = dynamic_cast<SkinnedMesh*>(loader2.getMesh());
		debugPrintDAE(daeModel_riggedFigure);

		StaticMeshActor* daeActor = spawnActor<StaticMeshActor>();
		daeActor->setStaticMesh(daeModel_riggedFigure);

		daeActor->setActorLocation(vector3(1.5f, 0.0f, 0.0f));
		daeActor->setActorRotation(Rotator(0.0f, 0.0f, 0.0f));
	} else {
		LOG(LogError, "Failed to load model: %s", FILE_RIGGED_FIGURE);
	}
}

void World2::onLoadWavefrontOBJ(OBJLoader* loader, uint64 payload)
{
	if (!loader->isValid()) {
		LOG(LogError, "Failed to load: %s", FILE_LPS_HEAD);
		return;
	}

	// #todo-sss: Implement subsurface scattering using this model.
	lpsHead = spawnActor<StaticMeshActor>();
	lpsHead->setStaticMesh(loader->craftMeshFromAllShapes(true));
	lpsHead->setActorLocation(0.0f, 10.0f, -10.0f);
	lpsHead->setActorScale(32.0f);
}
