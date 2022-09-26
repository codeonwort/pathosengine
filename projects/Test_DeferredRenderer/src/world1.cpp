#include "world1.h"
#include "player_controller.h"
#include "transform_test_actor.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/render/render_target.h"
#include "pathos/render/sky_atmosphere.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/gui/gui_window.h"
#include "pathos/scene/scene_capture_component.h"
#include "pathos/shader/material_shader.h"

// --------------------------------------------------------
// Constants

static const vector3 CAMERA_POSITION      = vector3(20.0f, 25.0f, 200.0f);
static const vector3 CAMERA_LOOK_AT       = vector3(20.0f, 25.0f, 190.0f);
static const vector3 SUN_DIRECTION        = glm::normalize(vector3(0.0f, -1.0f, -1.0f));
static const vector3 SUN_RADIANCE         = 1.2f * vector3(1.0f, 1.0f, 1.0f);

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
		vector3(600.0f, -10.0f, 0.0f),
		Rotator(-90.0f, 0.0f, 0.0f),
		vector3(50.0f)
	},
	{
		"breakfast_room/breakfast_room.obj",
		"breakfast_room/",
		vector3(-100.0f, -10.0f, 50.0f),
		Rotator(90.0f, 0.0f, 0.0f),
		vector3(30.0f)
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
		GLuint cubemapForIBL = IrradianceBaker::bakeCubemap(equirectangularMap, 512, "Texture IBL: cubemapForIBL");

		// diffuse irradiance
		{
			GLuint irradianceMap = IrradianceBaker::bakeIrradianceMap(cubemapForIBL, 32, false, "Texture IBL: diffuse irradiance");
			scene.irradianceMap = irradianceMap;
		}

		// specular IBL
		{
			GLuint prefilteredEnvMap;
			uint32 mipLevels;
			IrradianceBaker::bakePrefilteredEnvMap(cubemapForIBL, 128, prefilteredEnvMap, mipLevels, "Texture IBL: specular IBL (prefiltered env map)");

			scene.prefilterEnvMap = prefilteredEnvMap;
			scene.prefilterEnvMapMipLevels = mipLevels;
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

	Skybox* skybox = spawnActor<Skybox>();
	skybox->initialize(cubeTexture);
	skybox->setLOD(1.0f);
	scene.sky = skybox;
#elif SKY_METHOD == 1
	scene.sky = spawnActor<AtmosphereScattering>();
#elif SKY_METHOD == 2
	AnselSkyActor* ansel = spawnActor<AnselSkyActor>();
	GLuint anselTex = pathos::createTextureFromHDRImage(pathos::loadHDRImage(SKY_HDRI));
	ansel->initialize(anselTex);
	scene.sky = ansel;
#else
	GLuint hdri_temp = pathos::createTextureFromHDRImage(pathos::loadHDRImage(SKY_HDRI));
	Skybox* skybox = spawnActor<Skybox>();
	skybox->initialize(IrradianceBaker::bakeCubemap(hdri_temp, 512));
	scene.sky = skybox;
#endif
}

void World1::setupScene()
{
	// --------------------------------------------------------
	// Create materials

	GLuint tex = pathos::createTextureFromBitmap(loadImage("resources/textures/154.jpg"), true, true);
	GLuint tex_norm = pathos::createTextureFromBitmap(loadImage("resources/textures/154_norm.jpg"), true, false);

	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(0.9f, 0.2f, 0.2f);
		color->setMetallic(0.0f);
		color->setRoughness(0.1f);
	}
	Material* material_mirrorGround = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_mirrorGround);
		color->setAlbedo(0.9f, 0.9f, 0.9f);
		color->setMetallic(0.0f);
		color->setRoughness(0.0f);
	}

	// PBR material
	PBRTextureMaterial* material_pbr;
	{
		constexpr bool genMipmap = true;
		constexpr bool sRGB = true;
		BitmapBlob* albedoBlob = loadImage(SANDSTONE_ALBEDO);
		bool maskedMaterial = albedoBlob->hasOpacity;
		GLuint albedo = pathos::createTextureFromBitmap(albedoBlob, genMipmap, sRGB);
		GLuint normal = pathos::createTextureFromBitmap(loadImage(SANDSTONE_NORMAL), genMipmap, !sRGB);
		GLuint metallic = pathos::createTextureFromBitmap(loadImage(SANDSTONE_METALLIC), genMipmap, !sRGB);
		GLuint roughness = pathos::createTextureFromBitmap(loadImage(SANDSTONE_ROUGHNESS), genMipmap, !sRGB);
		GLuint ao = pathos::createTextureFromBitmap(loadImage(SANDSTONE_LOCAL_AO), genMipmap, !sRGB);

		material_pbr = new PBRTextureMaterial(albedo, normal, metallic, roughness, ao);
		material_pbr->writeAllPixels = !maskedMaterial;

		// #todo-material-assembler: Test if pbr_texture material works well
		material_pbr->bindMaterialShader(pathos::findMaterialShader("pbr_texture"));
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

	auto geom_sphere_big = new SphereGeometry(15.0f, 30);
	auto geom_sphere = new SphereGeometry(5.0f, 30);
	auto geom_plane = new PlaneGeometry(10.0f, 10.0f);
	auto geom_plane_big = new PlaneGeometry(10.0f, 10.0f, 20, 20);
	auto geom_cube = new CubeGeometry(vector3(5.0f));

	// --------------------------------------------------------
	// Lighting
	DirectionalLightActor* dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setLightParameters(SUN_DIRECTION, SUN_RADIANCE);

	PointLightActor* pointLight0 = spawnActor<PointLightActor>();
	PointLightActor* pointLight1 = spawnActor<PointLightActor>();
	PointLightActor* pointLight2 = spawnActor<PointLightActor>();
	PointLightActor* pointLight3 = spawnActor<PointLightActor>();

	pointLight0->setActorLocation(vector3(-50.0f + 700.0f, 60.0f, 170.0f));
	pointLight1->setActorLocation(vector3(0.0f + 700.0f, 30.0f, 150.0f));
	pointLight2->setActorLocation(vector3(-20.0f + 700.0f, 50.0f, 50.0f));
	pointLight3->setActorLocation(vector3(-20.0f + 700.0f, 50.0f, 150.0f));

	pointLight0->setLightParameters(50000.0f * vector3(0.2f, 2.0f, 1.0f), 100.0f, 0.001f, true);
	pointLight1->setLightParameters(30000.0f * vector3(2.0f, 0.2f, 1.0f), 100.0f, 0.001f, true);
	pointLight2->setLightParameters(50000.0f * vector3(2.0f, 0.0f, 0.0f), 150.0f, 0.001f, true);
	pointLight3->setLightParameters(20000.0f * vector3(2.0f, 2.0f, 2.0f), 200.0f, 0.0001f, true);

	godRaySource = spawnActor<StaticMeshActor>();
	godRaySource->setStaticMesh(new Mesh(geom_sphere, material_color));
	godRaySource->setActorScale(20.0f);
	godRaySource->setActorLocation(vector3(0.0f, 500.0f, -1500.0f));
	godRaySource->getStaticMeshComponent()->castsShadow = false;
	getScene().godRaySource = godRaySource->getStaticMeshComponent();

	// --------------------------------------------------------
	// Static meshes

	ground = spawnActor<StaticMeshActor>();
	ground->setStaticMesh(new Mesh(geom_plane_big, material_mirrorGround));
	ground->setActorScale(1000.0f);
	ground->setActorRotation(Rotator(0.0f, -90.0f, 0.0f));
	ground->setActorLocation(vector3(0.0f, -30.0f, 0.0f));
	ground->getStaticMeshComponent()->castsShadow = false;

	transformTestActor = spawnActor<TransformTestActor>();
	transformTestActor->setActorLocation(vector3(-800.0f, 50.0f, 100.0f));

	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		StaticMeshActor* ball = spawnActor<StaticMeshActor>();
		ball->setStaticMesh(new Mesh(geom_sphere, material_pbr));
		ball->setActorScale(5.0f + (float)i * 0.5f);
		ball->setActorLocation(vector3(-400.0f, 50.0f, 300.0f - 100.0f * i));
		balls.push_back(ball);
	}
	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		ColorMaterial* ball_material = new ColorMaterial;
		ball_material->setAlbedo(0.5f, 0.3f, 0.3f);
		ball_material->setMetallic(0.2f);
		ball_material->setRoughness((float)i / NUM_BALLS);

		StaticMeshActor* ball = spawnActor<StaticMeshActor>();
		ball->setStaticMesh(new Mesh(geom_cube, ball_material));
		ball->setActorScale(5.0f + (float)i * 0.5f);
		ball->setActorLocation(vector3(-550.0f, 50.0f, 300.0f - 100.0f * i));
		balls.push_back(ball);
	}

	constexpr float box_x0 = 250.0f;
	constexpr float box_y0 = 60.0f;
	constexpr float box_spaceX = 20.0f;
	constexpr float box_spaceY = 20.0f;
	float sinT = 0.0f;
	ColorMaterial* box_material = new ColorMaterial;
	box_material->setAlbedo(0.9f, 0.9f, 0.9f);
	box_material->setMetallic(0.0f);
	box_material->setRoughness(0.5f);
	for (uint32 i = 0; i < 16; ++i)
	{
		for (uint32 j = 0; j < 16; ++j)
		{
			float wave = ::sinf(sinT += 0.0417f);

			StaticMeshActor* box = spawnActor<StaticMeshActor>();
			box->setStaticMesh(new Mesh(geom_cube, box_material));
			box->setActorLocation(vector3(box_x0 + i * box_spaceX, 50.0f, box_y0 + j * box_spaceY));
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

	auto material_sceneCapture = PBRTextureMaterial::createWithFallback(tempRenderTarget->getGLName());
	StaticMeshActor* sceneCaptureViewer = spawnActor<StaticMeshActor>();
	sceneCaptureViewer->setStaticMesh(new Mesh(geom_plane, material_sceneCapture));
	sceneCaptureViewer->setActorLocation(-500.0f, 300.0f, -300.0f);
	sceneCaptureViewer->setActorScale(3.0f * vector3(16.0f, 9.0f, 1.0f));

	// --------------------------------------------------------
	// Bloom test
	ColorMaterial* material_tooBright = new ColorMaterial;
	material_tooBright->setAlbedo(0.0f, 0.0f, 0.0f);
	material_tooBright->setMetallic(0.0f);
	material_tooBright->setRoughness(1.0f);
	material_tooBright->setEmissive(100.0f, 0.0f, 0.0f);
	StaticMeshActor* bloomActor = spawnActor<StaticMeshActor>();
	bloomActor->setActorLocation(200.0f, 80.0f, 600.0f);
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

	for (Material* M : objModel->getStaticMesh()->getMaterials()) {
		ColorMaterial* CM = dynamic_cast<ColorMaterial*>(M);
		if (CM) {
			CM->setRoughness(1.0f);
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
		rot.yaw = fmod(rot.yaw + 1.0f, 180.0f);
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
		sprintf_s(title, "%s (GameThread: %.2f ms, RenderThread: %.2f ms, GPU: %.2f ms)",
			gEngine->getConfig().title,
			gEngine->getGameThreadCPUTime(),
			gEngine->getRenderThreadCPUTime(),
			gEngine->getGPUTime());
		gEngine->getMainWindow()->setTitle(title);
	}
}
