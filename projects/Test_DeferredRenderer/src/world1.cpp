#include "world1.h"
#include "player_controller.h"
#include "transform_test_actor.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"

#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/render/render_target.h"
#include "pathos/scene/scene_capture_component.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/gui/gui_window.h"

// --------------------------------------------------------
// Constants

static const vector3 CAMERA_POSITION      = vector3(0.0f, 1.0f, 10.0f);
static const vector3 CAMERA_LOOK_AT       = vector3(0.0f, 1.0f, 0.0f);
static const vector3 SUN_DIRECTION        = glm::normalize(vector3(-0.5f, -1.0f, 1.0f));
static const vector3 SUN_COLOR            = vector3(1.0f, 1.0f, 1.0f);
// Can't use real world lux as the pixel value of sun image in Ridgecrest_Road_Ref.hdr is 9.5
static const float   SUN_ILLUMINANCE      = 9.5f;
static const float   GOD_RAY_INTENSITY    = 5.0f;

// 0=skybox, 1=atmosphere, 2=panorama
#define              SKY_METHOD           2
static const char*   SKY_PANORAMA_HDRI    = "resources/skybox/HDRI/Ridgecrest_Road_Ref.hdr";

static const uint32  NUM_BALLS            = 10;
static const char*   SANDSTONE_ALBEDO     = "resources/textures/pbr_sandstone/sandstonecliff-albedo.png";
static const char*   SANDSTONE_NORMAL     = "resources/textures/pbr_sandstone/sandstonecliff-normal-ue.png";
static const char*   SANDSTONE_METALLIC   = "resources/textures/pbr_sandstone/sandstonecliff-metalness.png";
static const char*   SANDSTONE_ROUGHNESS  = "resources/textures/pbr_sandstone/sandstonecliff-roughness.png";
static const char*   SANDSTONE_LOCAL_AO   = "resources/textures/pbr_sandstone/sandstonecliff-ao.png";

static constexpr float  PILLAR_x0      = 2.5f;
static constexpr float  PILLAR_y0      = 0.5f;
static constexpr float  PILLAR_z0      = 0.6f;
static constexpr float  PILLAR_spaceX  = 0.2f;  // +x
static constexpr float  PILLAR_height  = 10.0f; // +y
static constexpr float  PILLAR_spaceZ  = 0.2f;  // +z
static constexpr uint32 PILLAR_columns = 16u;   // x-axis
static constexpr uint32 PILLAR_rows    = 16u;   // z-axis

// --------------------------------------------------------
// World

void World1::onInitialize()
{
	SCOPED_CPU_COUNTER(World1_initialize);

	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));

	gConsole->addLine(L"[world1]", false, true);
	gConsole->addLine(L"F : draw CSM frustum", false, true);
	gConsole->addLine(L"G : update scene capture", false, true);

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
	auto cubeImages = ImageUtils::loadCubemapImages(cubeImgName, ECubemapImagePreference::HLSL);
	Texture* cubeTexture = ImageUtils::createTextureCubeFromImages(cubeImages, 0, true, "Texture_SkyCubemap");
	SkyboxActor* skybox = spawnActor<SkyboxActor>();
	skybox->setCubemapTexture(cubeTexture);
#elif SKY_METHOD == 1
	SkyAtmosphereActor* skyAtmosphere = spawnActor<SkyAtmosphereActor>();
#elif SKY_METHOD == 2
	PanoramaSkyActor* panoramaSky = spawnActor<PanoramaSkyActor>();
	ImageBlob* panoramaBlob = ImageUtils::loadImage(SKY_PANORAMA_HDRI);
	Texture* panoramaTex = ImageUtils::createTexture2DFromImage(panoramaBlob, 1, false, true, "Texture_Panorama");
	panoramaSky->setTexture(panoramaTex);
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
		constexpr uint32 mipLevels = 0;
		constexpr bool sRGB = true;
		Texture* albedo = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(SANDSTONE_ALBEDO), mipLevels, sRGB);
		Texture* normal = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(SANDSTONE_NORMAL), mipLevels, !sRGB);
		Texture* metallic = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(SANDSTONE_METALLIC), mipLevels, !sRGB);
		Texture* roughness = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(SANDSTONE_ROUGHNESS), mipLevels, !sRGB);
		Texture* ao = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(SANDSTONE_LOCAL_AO), mipLevels, !sRGB);

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
	dirLight->setColorAndIlluminance(SUN_COLOR, SUN_ILLUMINANCE);

	PointLightActor* pointLight0 = spawnActor<PointLightActor>();
	PointLightActor* pointLight1 = spawnActor<PointLightActor>();
	PointLightActor* pointLight2 = spawnActor<PointLightActor>();

	const float PILLAR_x1 = PILLAR_x0 + PILLAR_spaceX * PILLAR_columns;
	const float PILLAR_y1 = PILLAR_y0 - 0.01f;
	const float PILLAR_z1 = PILLAR_z0 + PILLAR_spaceZ * PILLAR_rows;
	pointLight0->setActorLocation(vector3(PILLAR_x0, PILLAR_y1, PILLAR_z0));
	pointLight1->setActorLocation(vector3(PILLAR_x1, PILLAR_y1, PILLAR_z0));
	pointLight2->setActorLocation(vector3(0.5f * (PILLAR_x0 + PILLAR_x1), PILLAR_y1, PILLAR_z1));

	pointLight0->setColorAndIntensity(vector3(1.0f, 0.0f, 0.0f), 50.0f);
	pointLight0->setAttenuationRadius(2.0f);

	pointLight1->setColorAndIntensity(vector3(0.0f, 1.0f, 0.0f), 50.0f);
	pointLight1->setAttenuationRadius(2.0f);

	pointLight2->setColorAndIntensity(vector3(0.0f, 0.0f, 1.0f), 50.0f);
	pointLight2->setAttenuationRadius(2.0f);

	godRaySource = spawnActor<StaticMeshActor>();
	godRaySource->setStaticMesh(new Mesh(geom_sphere, material_color));
	godRaySource->setActorScale(20.0f);
	godRaySource->setActorLocation(vector3(0.0f, 5.0f, -15.0f));
	godRaySource->getStaticMeshComponent()->castsShadow = false;
	getScene().godRaySource = godRaySource->getStaticMeshComponent();
	getScene().godRayIntensity = GOD_RAY_INTENSITY;

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

	float sinT = 0.0f;
	Material* box_material = Material::createMaterialInstance("solid_color");

	box_material->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	box_material->setConstantParameter("metallic", 0.0f);
	box_material->setConstantParameter("roughness", 0.5f);
	box_material->setConstantParameter("emissive", vector3(0.0f));
	for (uint32 i = 0; i < PILLAR_columns; ++i)
	{
		for (uint32 j = 0; j < PILLAR_rows; ++j)
		{
			float wave = ::sinf(sinT += 0.0417f);

			StaticMeshActor* pillar = spawnActor<StaticMeshActor>();
			pillar->setStaticMesh(new Mesh(geom_cube, box_material));
			pillar->setActorLocation(vector3(PILLAR_x0 + i * PILLAR_spaceX, PILLAR_y0, PILLAR_z0 + j * PILLAR_spaceZ));
			pillar->setActorScale(vector3(1.0f, PILLAR_height * 0.5f * (1.0f + wave), 1.0f));

			pillars.push_back(pillar);
		}
	}

	// --------------------------------------------------------
	// Scene capture test
	if (tempRenderTarget == nullptr) {
		tempRenderTarget = new RenderTarget2D;
		tempRenderTarget->respecTexture(1920, 1080, RenderTargetFormat::RGBA16F);
		tempRenderTarget->immediateUpdateResource();
	}
	if (sceneCaptureComponent == nullptr) {
		sceneCaptureComponent = new SceneCaptureComponent;
		sceneCaptureComponent->renderTarget = tempRenderTarget;
	}
	if (sceneCaptureActor == nullptr) {
		sceneCaptureActor = spawnActor<Actor>();
		sceneCaptureActor->registerComponent(sceneCaptureComponent);
		sceneCaptureActor->setActorLocation(CAMERA_POSITION);
	}
	sceneCaptureComponent->captureScene();

	Material* material_sceneCapture = Material::createMaterialInstance("texture_viewer");
	material_sceneCapture->setTextureParameter("inputTexture", tempRenderTarget->getInternalTexture());
	
	StaticMeshActor* sceneCaptureViewer = spawnActor<StaticMeshActor>();
	sceneCaptureViewer->setStaticMesh(new Mesh(geom_sceneCapture, material_sceneCapture));
	sceneCaptureViewer->setActorLocation(-5.0f, 5.0f, -3.0f);
	sceneCaptureViewer->setActorScale(0.5f * vector3(16.0f, 9.0f, 1.0f));

	// --------------------------------------------------------
	// Bloom test
	Material* material_tooBright = Material::createMaterialInstance("solid_color");
	material_tooBright->setConstantParameter("albedo", vector3(0.0f, 0.0f, 0.0f));
	material_tooBright->setConstantParameter("metallic", 0.0f);
	material_tooBright->setConstantParameter("roughness", 1.0f);
	material_tooBright->setConstantParameter("emissive", vector3(100.0f, 0.0f, 0.0f));
	StaticMeshActor* bloomActor = spawnActor<StaticMeshActor>();
	bloomActor->setActorLocation(1.0f, 0.8f, -1.0f);
	bloomActor->setActorScale(20.0f);
	bloomActor->setStaticMesh(new Mesh(geom_sphere, material_tooBright));
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
	for (uint32 i = 0; i < PILLAR_columns; ++i)
	{
		for (uint32 j = 0; j < PILLAR_rows; ++j)
		{
			float wave = ::sinf(sinT + 13.2754f * (i * 16 + j) / 256.0f + (6.3f * j / 16.0f));
			pillars[i * 16 + j]->setActorScale(vector3(0.5f, PILLAR_height * 0.5f * (1.0f + wave), 0.5f));
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
