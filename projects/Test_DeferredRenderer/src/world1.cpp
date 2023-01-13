#include "world1.h"
#include "player_controller.h"
#include "transform_test_actor.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/render/render_target.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/gui/gui_window.h"
#include "pathos/scene/scene_capture_component.h"
#include "pathos/scene/sky_ansel_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/reflection_probe_actor.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/material/material_shader.h"

// --------------------------------------------------------
// Constants

static const vector3 CAMERA_POSITION      = vector3(0.0f, 1.0f, 2.0f);
static const vector3 CAMERA_LOOK_AT       = vector3(0.0f, 1.0f, 0.0f);
static const vector3 SUN_DIRECTION        = glm::normalize(vector3(-0.5f, -1.0f, 1.0f));
static const vector3 SUN_ILLUMINANCE      = 5.0f * vector3(1.0f, 1.0f, 1.0f);

#define              SKY_METHOD           2
static const char*   SKY_HDRI             = "resources/skybox/HDRI/Ridgecrest_Road_Ref.hdr";

static const uint32  NUM_BALLS            = 10;
static const char*   SANDSTONE_ALBEDO     = "resources/textures/pbr_sandstone/sandstonecliff-albedo.png";
static const char*   SANDSTONE_NORMAL     = "resources/textures/pbr_sandstone/sandstonecliff-normal-ue.png";
static const char*   SANDSTONE_METALLIC   = "resources/textures/pbr_sandstone/sandstonecliff-metalness.png";
static const char*   SANDSTONE_ROUGHNESS  = "resources/textures/pbr_sandstone/sandstonecliff-roughness.png";
static const char*   SANDSTONE_LOCAL_AO   = "resources/textures/pbr_sandstone/sandstonecliff-ao.png";

struct WavefrontModelDesc {
	std::string filepath;
	std::string dir;
	vector3 location;
	Rotator rotation;
	vector3 scale;
};

std::vector<WavefrontModelDesc> wavefrontModels = {
	{
		"models/fireplace_room/fireplace_room.obj",
		"models/fireplace_room/",
		vector3(6.0f, 0.0f, 0.0f),
		Rotator(-90.0f, 0.0f, 0.0f),
		vector3(0.5f)
	},
	{
		"breakfast_room/breakfast_room.obj",
		"breakfast_room/",
		vector3(-1.0f, 0.2f, 0.5f),
		Rotator(90.0f, 0.0f, 0.0f),
		vector3(0.3f)
	},
};

// --------------------------------------------------------
// World

void World1::onInitialize()
{
	SCOPED_CPU_COUNTER(World1_initialize);

	for (auto i = 0u; i < wavefrontModels.size(); ++i) {
		AssetReferenceWavefrontOBJ assetRef(
			wavefrontModels[i].filepath.c_str(),
			wavefrontModels[i].dir.c_str());
		gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRef, this, &World1::onLoadOBJ, i);
	}

	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));

	setupInput();
	setupSky();
	setupScene();
}

void World1::setupInput()
{
	playerController = spawnActor<PlayerController>();

	ButtonBinding drawShadowFrustum;
	drawShadowFrustum.addInput(InputConstants::KEYBOARD_F);

	ButtonBinding updateSceneCapture;
	updateSceneCapture.addInput(InputConstants::KEYBOARD_G);

	InputManager* inputManager = getInputManager();
	inputManager->bindButtonPressed("drawShadowFrustum", drawShadowFrustum, [this]()
		{
			setupCSMDebugger();
		}
	);
	inputManager->bindButtonPressed("updateSceneCapture", updateSceneCapture, [this]()
		{
			if (sceneCaptureComponent != nullptr) {
				sceneCaptureComponent->setLocation(camera.getPosition());
				sceneCaptureComponent->setRotation(Rotator(camera.getYaw(), camera.getPitch(), 0.0f));
				sceneCaptureComponent->captureScene();
			}
		}
	);
}

void World1::setupSky()
{
	{
		GLuint equirectangularMap = pathos::createTextureFromHDRImage(pathos::loadHDRImage(SKY_HDRI), true, "Texture IBL: equirectangularMap");
		GLuint cubemapForIBL = ImageBasedLightingBaker::projectToCubemap(equirectangularMap, 512, "Texture IBL: cubemapForIBL");

		// diffuse irradiance
		{
			GLuint irradianceMap = ImageBasedLightingBaker::bakeSkyIrradianceMap(cubemapForIBL, 32, false, "Texture IBL: diffuse irradiance");
			scene.skyIrradianceMap = irradianceMap;
		}

		// specular IBL
		{
			GLuint prefilteredEnvMap;
			uint32 mipLevels;
			ImageBasedLightingBaker::bakeSkyPrefilteredEnvMap(cubemapForIBL, 128, prefilteredEnvMap, mipLevels, "Texture IBL: specular IBL (prefiltered env map)");

			scene.skyPrefilterEnvMap = prefilteredEnvMap;
			scene.skyPrefilterEnvMapMipLevels = mipLevels;
		}
	}

	// --------------------------------------------------------
	// Sky
#if SKY_METHOD == 0
#if DEBUG_SKYBOX
	std::array<const char*, 6> cubeImgName = {
		"resources/skybox/placeholder/cubemap_right.jpg",
		"resources/skybox/placeholder/cubemap_left.jpg",
		"resources/skybox/placeholder/cubemap_top.jpg",
		"resources/skybox/placeholder/cubemap_bottom.jpg",
		"resources/skybox/placeholder/cubemap_front.jpg",
		"resources/skybox/placeholder/cubemap_back.jpg"
	};
#else
	std::array<const char*, 6> cubeImgName = {
		"resources/skybox/cubemap1/pos_x.jpg",
		"resources/skybox/cubemap1/neg_x.jpg",
		"resources/skybox/cubemap1/pos_y.jpg",
		"resources/skybox/cubemap1/neg_y.jpg",
		"resources/skybox/cubemap1/pos_z.jpg",
		"resources/skybox/cubemap1/neg_z.jpg"
	};
#endif
	std::array<BitmapBlob*, 6> cubeImg;
	pathos::loadCubemapImages(cubeImgName, ECubemapImagePreference::HLSL, cubeImg);
	GLuint cubeTexture = pathos::createCubemapTextureFromBitmap(cubeImg.data(), true, "skybox cubemap");

	SkyboxActor* skybox = spawnActor<SkyboxActor>();
	skybox->initialize(cubeTexture);
	skybox->setLOD(1.0f);
	scene.sky = skybox;
#elif SKY_METHOD == 1
	scene.sky = spawnActor<SkyAtmosphereActor>();
#elif SKY_METHOD == 2
	AnselSkyActor* ansel = spawnActor<AnselSkyActor>();
	GLuint anselTex = pathos::createTextureFromHDRImage(pathos::loadHDRImage(SKY_HDRI));
	ansel->initialize(anselTex);
	scene.sky = ansel;
#else
	GLuint hdri_temp = pathos::createTextureFromHDRImage(pathos::loadHDRImage(SKY_HDRI));
	SkyboxActor* skybox = spawnActor<SkyboxActor>();
	skybox->initialize(ImageBasedLightingBaker::projectToCubemap(hdri_temp, 512));
	scene.sky = skybox;
#endif
}

void World1::setupScene()
{
	// --------------------------------------------------------
	// Create materials

	Material* material_color = Material::createMaterialInstance("solid_color");
	material_color->setConstantParameter("albedo", vector3(0.9f, 0.2f, 0.2f));
	material_color->setConstantParameter("metallic", 0.0f);
	material_color->setConstantParameter("roughness", 0.1f);
	material_color->setConstantParameter("emissive", vector3(0.0f));

	Material* material_mirrorGround = Material::createMaterialInstance("solid_color");
	material_mirrorGround->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	material_mirrorGround->setConstantParameter("metallic", 0.0f);
	material_mirrorGround->setConstantParameter("roughness", 0.1f);
	material_mirrorGround->setConstantParameter("emissive", vector3(0.0f));

	// PBR material
	Material* material_pbr = Material::createMaterialInstance("pbr_texture");
	{
		constexpr bool genMipmap = true;
		constexpr bool sRGB = true;
		BitmapBlob* albedoBlob = loadImage(SANDSTONE_ALBEDO);
		GLuint albedo = pathos::createTextureFromBitmap(albedoBlob, genMipmap, sRGB);
		GLuint normal = pathos::createTextureFromBitmap(loadImage(SANDSTONE_NORMAL), genMipmap, !sRGB);
		GLuint metallic = pathos::createTextureFromBitmap(loadImage(SANDSTONE_METALLIC), genMipmap, !sRGB);
		GLuint roughness = pathos::createTextureFromBitmap(loadImage(SANDSTONE_ROUGHNESS), genMipmap, !sRGB);
		GLuint ao = pathos::createTextureFromBitmap(loadImage(SANDSTONE_LOCAL_AO), genMipmap, !sRGB);

		material_pbr->setConstantParameter("bOverrideAlbedo", false);
		material_pbr->setConstantParameter("bOverrideNormal", false);
		material_pbr->setConstantParameter("bOverrideMetallic", false);
		material_pbr->setConstantParameter("bOverrideRoughness", false);
		material_pbr->setConstantParameter("bOverrideLocalAO", false);
		material_pbr->setConstantParameter("emissiveConstant", vector3(0.0f));
		material_pbr->setTextureParameter("albedo", albedo);
		material_pbr->setTextureParameter("normal", normal);
		material_pbr->setTextureParameter("metallic", metallic);
		material_pbr->setTextureParameter("roughness", roughness);
		material_pbr->setTextureParameter("localAO", ao);
	}

	// --------------------------------------------------------
	// Create geometries

	auto geom_sphere_big = new SphereGeometry(0.15f, 30);
	auto geom_sphere = new SphereGeometry(0.05f, 30);
	auto geom_sceneCapture = new PlaneGeometry(1.0f, 1.0f);
	auto geom_ground = new PlaneGeometry(100.0f, 100.0f, 4, 4);
	auto geom_cube = new CubeGeometry(vector3(0.05f));

	// --------------------------------------------------------
	// Lighting
	DirectionalLightActor* dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setDirection(SUN_DIRECTION);
	dirLight->setIlluminance(SUN_ILLUMINANCE);

	PointLightActor* pointLight0 = spawnActor<PointLightActor>();
	PointLightActor* pointLight1 = spawnActor<PointLightActor>();
	PointLightActor* pointLight2 = spawnActor<PointLightActor>();
	PointLightActor* pointLight3 = spawnActor<PointLightActor>();

	pointLight0->setActorLocation(vector3(-0.5f + 7.0f, 0.6f, 1.7f));
	pointLight1->setActorLocation(vector3(00.0f + 7.0f, 0.3f, 1.5f));
	pointLight2->setActorLocation(vector3(-0.2f + 7.0f, 0.5f, 0.5f));
	pointLight3->setActorLocation(vector3(-0.2f + 7.0f, 0.5f, 1.5f));

	pointLight0->setIntensity(5.0f * vector3(0.2f, 2.0f, 1.0f));
	pointLight0->setAttenuationRadius(1.0f);

	pointLight1->setIntensity(4.0f * vector3(0.1f, 0.1f, 1.0f));
	pointLight1->setAttenuationRadius(1.0f);

	pointLight2->setIntensity(10.0f * vector3(1.0f, 0.0f, 0.0f));
	pointLight2->setAttenuationRadius(1.5f);

	pointLight3->setIntensity(3.0f * vector3(1.0f, 1.0f, 1.0f));
	pointLight3->setAttenuationRadius(2.0f);

	godRaySource = spawnActor<StaticMeshActor>();
	godRaySource->setStaticMesh(new Mesh(geom_sphere, material_color));
	godRaySource->setActorScale(20.0f);
	godRaySource->setActorLocation(vector3(0.0f, 5.0f, -15.0f));
	godRaySource->getStaticMeshComponent()->castsShadow = false;
	getScene().godRaySource = godRaySource->getStaticMeshComponent();

	// --------------------------------------------------------
	// Static meshes

	ground = spawnActor<StaticMeshActor>();
	ground->setStaticMesh(new Mesh(geom_ground, material_mirrorGround));
	ground->setActorRotation(Rotator(0.0f, -90.0f, 0.0f));
	ground->setActorLocation(vector3(0.0f, -0.3f, 0.0f));
	ground->getStaticMeshComponent()->castsShadow = false;

	transformTestActor = spawnActor<TransformTestActor>();
	transformTestActor->setActorLocation(vector3(-8.0f, 0.5f, 1.0f));

	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		StaticMeshActor* ball = spawnActor<StaticMeshActor>();
		ball->setStaticMesh(new Mesh(geom_sphere, material_pbr));
		ball->setActorScale(5.0f + (float)i * 0.5f);
		ball->setActorLocation(vector3(-4.0f, 0.5f, 3.0f - 1.0f * i));
		balls.push_back(ball);
	}
	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		Material* ball_material = Material::createMaterialInstance("solid_color");
		ball_material->setConstantParameter("albedo", vector3(0.5f, 0.3f, 0.3f));
		ball_material->setConstantParameter("metallic", 0.0f);
		ball_material->setConstantParameter("roughness", (float)i / NUM_BALLS);
		ball_material->setConstantParameter("emissive", vector3(0.0f));

		StaticMeshActor* ball = spawnActor<StaticMeshActor>();
		ball->setStaticMesh(new Mesh(geom_cube, ball_material));
		ball->setActorScale(5.0f + (float)i * 0.5f);
		ball->setActorLocation(vector3(-5.5f, 0.5f, 3.0f - 1.0f * i));
		balls.push_back(ball);
	}

	constexpr float box_x0 = 2.5f;
	constexpr float box_y0 = 0.6f;
	constexpr float box_spaceX = 0.2f;
	constexpr float box_spaceY = 0.2f;
	float sinT = 0.0f;
	Material* box_material = Material::createMaterialInstance("solid_color");

	box_material->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	box_material->setConstantParameter("metallic", 0.0f);
	box_material->setConstantParameter("roughness", 0.5f);
	box_material->setConstantParameter("emissive", vector3(0.0f));
	for (uint32 i = 0; i < 16; ++i)
	{
		for (uint32 j = 0; j < 16; ++j)
		{
			float wave = ::sinf(sinT += 0.0417f);

			StaticMeshActor* box = spawnActor<StaticMeshActor>();
			box->setStaticMesh(new Mesh(geom_cube, box_material));
			box->setActorLocation(vector3(box_x0 + i * box_spaceX, 0.5f, box_y0 + j * box_spaceY));
			box->setActorScale(vector3(1.0f, 10.0f * 0.5f * (1.0f + wave), 1.0f));

			//if (i == 0 && j == 0) {
			//	box->setActorLocation(0.0f, 20.0f, 180.0f);
			//}

			boxes.push_back(box);
		}
	}

	// --------------------------------------------------------
	// Scene capture test
	if (tempRenderTarget == nullptr) {
		tempRenderTarget = new RenderTarget2D;
		tempRenderTarget->respecTexture(1920, 1080, RenderTargetFormat::RGBA16F);
		tempRenderTarget->immediateUpdateResource();
	}

	if (sceneCaptureActor == nullptr) {
		sceneCaptureActor = spawnActor<Actor>();
		sceneCaptureComponent = new SceneCaptureComponent;
		sceneCaptureActor->registerComponent(sceneCaptureComponent);

		sceneCaptureComponent->renderTarget = tempRenderTarget;

		sceneCaptureActor->setActorLocation(CAMERA_POSITION);
	}
	sceneCaptureComponent->captureScene();

	Material* material_sceneCapture = Material::createMaterialInstance("texture_viewer");
	material_sceneCapture->setTextureParameter("inputTexture", tempRenderTarget->getGLName());
	
	StaticMeshActor* sceneCaptureViewer = spawnActor<StaticMeshActor>();
	sceneCaptureViewer->setStaticMesh(new Mesh(geom_sceneCapture, material_sceneCapture));
	sceneCaptureViewer->setActorLocation(-5.0f, 3.0f, -3.0f);
	sceneCaptureViewer->setActorScale(0.1f * vector3(16.0f, 9.0f, 1.0f));

	// --------------------------------------------------------
	// Bloom test
	Material* material_tooBright = Material::createMaterialInstance("solid_color");
	material_tooBright->setConstantParameter("albedo", vector3(0.0f, 0.0f, 0.0f));
	material_tooBright->setConstantParameter("metallic", 0.0f);
	material_tooBright->setConstantParameter("roughness", 1.0f);
	material_tooBright->setConstantParameter("emissive", vector3(100.0f, 0.0f, 0.0f));
	StaticMeshActor* bloomActor = spawnActor<StaticMeshActor>();
	bloomActor->setActorLocation(2.0f, 0.8f, 6.0f);
	bloomActor->setActorScale(20.0f);
	bloomActor->setStaticMesh(new Mesh(geom_sphere, material_tooBright));
}

void World1::onLoadOBJ(OBJLoader* loader, uint64 payload)
{
	const WavefrontModelDesc& desc = wavefrontModels[payload];

	if (!loader->isValid()) {
		LOG(LogError, "Failed to load a wavefront model: %s", desc.filepath.c_str());
		return;
	}

	StaticMeshActor* objModel = spawnActor<StaticMeshActor>();
	objModel->setStaticMesh(loader->craftMeshFromAllShapes());
	objModel->setActorRotation(desc.rotation);
	objModel->setActorScale(desc.scale);
	objModel->setActorLocation(desc.location);

	if (payload == 1) {
		objModel->getStaticMeshComponent()->updateTransformHierarchy();
		AABB worldBounds = objModel->getStaticMeshComponent()->getWorldBounds();
		if (payload == 0) {
			worldBounds.minBounds += vector3(0.4f, 0.2f, 0.2f);
			worldBounds.maxBounds -= vector3(0.2f, 0.2f, 0.4f);
		} else {
			worldBounds.minBounds += vector3(0.2f, 0.2f, 0.2f);
			worldBounds.maxBounds -= vector3(0.2f, 0.2f, 0.2f);
		}
		const vector3ui GRID_SIZE(4, 4, 4);

		IrradianceVolumeActor* volume = spawnActor<IrradianceVolumeActor>();
		volume->initializeVolume(worldBounds.minBounds, worldBounds.maxBounds, GRID_SIZE);

		//LOG(LogDebug, "volume bounds: min(%.3f, %.3f, %.3f)", worldBounds.minBounds.x, worldBounds.minBounds.y, worldBounds.minBounds.z);
		//LOG(LogDebug, "volume bounds: max(%.3f, %.3f, %.3f)", worldBounds.maxBounds.x, worldBounds.maxBounds.y, worldBounds.maxBounds.z);
		//for (uint32 i = 0; i < volume->numProbes(); ++i) {
		//	vector3 pos = volume->getProbeLocationByIndex(i);
		//	LOG(LogDebug, "probe: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
		//}
	}

	for (Material* M : objModel->getStaticMesh()->getMaterials()) {
		if (M->getMaterialName() == "solid_color") {
			M->setConstantParameter("roughness", 1.0f);
		}
	}

	objModels.push_back(objModel);
}

void World1::setupCSMDebugger()
{
#if VISUALIZE_CSM_FRUSTUM
	if (csmDebugger == nullptr) {
		csmDebugger = spawnActor<CSMDebugger>();
	}
	csmDebugger->drawCameraFrustum(camera, SUN_DIRECTION);
#endif
}

void World1::onTick(float deltaSeconds)
{
	static float ballAngle = 0.0f;
	for (StaticMeshActor* ball : balls) {
		Rotator rot = ball->getActorRotation();
		rot.yaw = fmod(rot.yaw + 1.0f, 360.0f);
		ball->setActorRotation(rot);
	}

	static float sinT = 0.0f;
	sinT += 6.28f * 0.25f * deltaSeconds;
	for (uint32 i = 0; i < 16; ++i)
	{
		for (uint32 j = 0; j < 16; ++j)
		{
			float wave = ::sinf(sinT + 13.2754f * (i * 16 + j) / 256.0f + (6.3f * j / 16.0f));
			boxes[i * 16 + j]->setActorScale(vector3(0.5f, 10.0f * 0.5f * (1.0f + wave), 0.5f));
		}
	}

#if 0 // lookat debug
	static float lookAtTime = 0.0f;
	lookAtTime += 6.28f * deltaSeconds * 0.1f;
	vector3 lookDir(100.0f * sinf(lookAtTime), 0.0f, 100.0f * cosf(lookAtTime));
	camera.lookAt(CAMERA_POSITION, CAMERA_POSITION + lookDir, vector3(0.0f, 1.0f, 0.0f));
#endif

	{
		char title[256];
		sprintf_s(title, "Some Demo World (GameThread: %.2f ms, RenderThread: %.2f ms, GPU: %.2f ms)",
			gEngine->getGameThreadCPUTime(),
			gEngine->getRenderThreadCPUTime(),
			gEngine->getGPUTime());
		gEngine->getMainWindow()->setTitle(title);
	}
}
