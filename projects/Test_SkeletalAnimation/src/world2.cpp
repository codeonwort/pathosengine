#include "world2.h"
#include "daeloader.h"
#include "skinned_mesh.h"
#include "player_controller.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/gui/gui_window.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/skybox_actor.h"
#include "pathos/scene/reflection_probe_actor.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/text/text_actor.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/render/image_based_lighting_baker.h"

#include <time.h>

#define FOV_Y                             60.0f
#define CAMERA_POSITION                   vector3(0.0f, 2.0f, 6.0f)
#define CAMERA_LOOK_AT                    vector3(0.0f, 2.0f, 4.0f)

#define SUN_DIRECTION                     glm::normalize(vector3(0.0f, -1.0f, -1.0f))
#define SUN_COLOR                         vector3(1.0f, 1.0f, 1.0f)
// #wip: Check light intensities
// Too bright for luminance histogram. What's a proper value for minLogLuiminance?
// 
// Clear blue sky : 20000 lux
// Overcast day   : 1000 lux
// The skybox is full of clouds but not ~95%, so 3000 lux would it be?
#define SUN_ILLUMINANCE                   3000.0f
#define SKY_INTENSITY_MULTIPLIER          300.0f

#define GOD_RAY_COLOR                     vector3(0.9f, 0.5f, 0.0f)
#define GOD_RAY_INTENSITY                 10000.0f

#define POINT_LIGHT_LOCATION              vector3(0.0f, 2.0f, 2.0f)
#define POINT_LIGHT_COLOR                 vector3(0.2f, 1.0f, 0.2f)
#define POINT_LIGHT_INTENSITY             50000.0f
#define POINT_LIGHT_ATTENUATION_RADIUS    10.0f
#define POINT_LIGHT_SOURCE_RADIUS         0.2f

#define DIR_MY_ANIMTEST                   "resources/models/animtest/"
#define FILE_MY_ANIMTEST                  "resources/models/animtest/animtest.dae"
#define DIR_RIGGED_FIGURE                 "resources_external/KhronosGroup/RiggedFigure/"
#define FILE_RIGGED_FIGURE                "resources_external/KhronosGroup/RiggedFigure/RiggedFigure.dae"
#define FILE_LPS_HEAD                     "resources_external/LPSHead/head.obj"
#define DIR_LPS_HEAD                      "resources_external/LPSHead/"
#define DOWNLOAD_ALERT_MSG1               L"If you can't see 2 animated models, run Setup.ps1"
#define DOWNLOAD_ALERT_MSG2               L"움직이는 모델 2개가 표시되지 않으면 Setup.ps1을 실행해주세요"


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

	//---------------------------------------------------------------------------------------
	// Light sources

	sunLight = spawnActor<DirectionalLightActor>();
	sunLight->setDirection(SUN_DIRECTION);
	sunLight->setColorAndIlluminance(SUN_COLOR, SUN_ILLUMINANCE);

	pointLight0 = spawnActor<PointLightActor>();
	pointLight0->setActorLocation(POINT_LIGHT_LOCATION);
	pointLight0->setColorAndIntensity(POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
	pointLight0->setAttenuationRadius(POINT_LIGHT_ATTENUATION_RADIUS);
	pointLight0->setSourceRadius(POINT_LIGHT_SOURCE_RADIUS);

	//---------------------------------------------------------------------------------------
	// Local light probes

	vector3 reflectionProbeLocations[] = {
		vector3(0.0f, 4.0f, 3.0f),
		vector3(15.0f, 5.0f, -10.0f),
		vector3(-15.0f, 5.0f, -10.0f),
		vector3(0.0f, 7.0f, -20.0f),
	};
	for (size_t i = 0; i < _countof(reflectionProbeLocations); ++i) {
		auto probe = spawnActor<ReflectionProbeActor>();
		probe->setActorLocation(reflectionProbeLocations[i]);
	}

	IrradianceVolumeActor* irradianceVolume = spawnActor<IrradianceVolumeActor>();
	irradianceVolume->initializeVolume(
		vector3(-20.0f, 1.0f, -30.0f),
		vector3(20.0f, 20.0f, 10.0f),
		vector3ui(5, 5, 7));

	//---------------------------------------------------------------------------------------
	// Materials

	std::array<const char*, 6> cubeImageNames = {
		"skybox/cubemap1/pos_x.jpg", "skybox/cubemap1/neg_x.jpg",
		"skybox/cubemap1/pos_y.jpg", "skybox/cubemap1/neg_y.jpg",
		"skybox/cubemap1/pos_z.jpg", "skybox/cubemap1/neg_z.jpg"
	};
	auto cubeImageBlobs = ImageUtils::loadCubemapImages(cubeImageNames, ECubemapImagePreference::HLSL);
	Texture* skyCubemapTexture = ImageUtils::createTextureCubeFromImages(cubeImageBlobs, 0, true, "Texture_Skybox");

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
	sky->setCubemapTexture(skyCubemapTexture);
	sky->setIntensityMultiplier(SKY_INTENSITY_MULTIPLIER);

	scene.godRaySource = godRaySourceMesh->getStaticMeshComponent();
	scene.godRayColor = GOD_RAY_COLOR;
	scene.godRayIntensity = GOD_RAY_INTENSITY;
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
