#include "world_game1.h"
#include "player_controller.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/skybox_actor.h"
#include "pathos/scene/sky_ansel_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/volumetric_cloud_actor.h"
#include "pathos/loader/scene_loader.h"
#include "pathos/input/input_manager.h"

#define SCENE_DESC_FILE "resources/racing_game/test_scene.json"

#define CLOUD_WEATHER_MAP_FILE   "racing_game/WeatherMap.png"
#define CLOUD_SHAPE_NOISE_FILE   "common/noiseShapePacked.tga"
#define CLOUD_EROSION_NOISE_FILE "common/noiseErosionPacked.tga"

const vector3 CAMERA_POSITION = vector3(0.0f, 0.0f, 50.0f);
const vector3 CAMERA_LOOK_AT  = vector3(0.0f, 0.0f, 0.0f);

World_Game1::World_Game1()
{
}

void World_Game1::onInitialize()
{
	SCOPED_CPU_COUNTER(World_Game1_initialize);

	prepareAssets();
	reloadScene();

	gEngine->registerExec("reload_scene", [this](const std::string& command) {
		reloadScene();
	});

	ButtonBinding photoMode;
	photoMode.addInput(InputConstants::KEYBOARD_P);

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindButtonPressed("photoMode", photoMode, [this]() {
		playerController->togglePhotoMode();
	});

	gConsole->addLine("Press 'WASD' to control the pawn");
	gConsole->addLine("Press 'P' to toggle photo mode");
}

void World_Game1::onTick(float deltaSeconds)
{
	//vector3 loc = pointLight0->getActorLocation();
	//loc.x = 10.0f * ::sinf(gEngine->getWorldTime());
	//pointLight0->setActorLocation(loc);
	//PointLightComponent* p = static_cast<PointLightComponent*>(pointLight0->getRootComponent());
	//p->color.g = (1.0f + ::cosf(gEngine->getWorldTime())) * 10.0f;
}

void World_Game1::prepareAssets()
{
	Material* M_color = Material::createMaterialInstance("solid_color");
	M_color->setConstantParameter("albedo", vector3(0.9f, 0.1f, 0.1f));
	M_color->setConstantParameter("metallic", 0.0f);
	M_color->setConstantParameter("roughness", 0.2f);
	M_color->setConstantParameter("emissive", vector3(0.0f));

	GLuint landscapeAlbedo = pathos::createTextureFromBitmap(pathos::loadImage("resources/racing_game/landscape.jpg"), true, true);
	Material* M_landscape = pathos::createPBRMaterial(landscapeAlbedo);

	auto G_sphere = new SphereGeometry(1.0f, 30);
	auto G_plane = new PlaneGeometry(128.0f, 128.0f, 1, 1);

	sphereMesh = new Mesh(G_sphere, M_color);
	landscapeMesh = new Mesh(G_plane, M_landscape);

	weatherTexture = pathos::createTextureFromBitmap(pathos::loadImage(CLOUD_WEATHER_MAP_FILE), true, false, "Texture: WeatherMap");
	cloudShapeNoise = pathos::loadVolumeTextureFromTGA(CLOUD_SHAPE_NOISE_FILE, "Texture_CloudShapeNoise");
	{
		uint32 vtWidth = cloudShapeNoise->getSourceImageWidth();
		uint32 vtHeight = cloudShapeNoise->getSourceImageHeight();
		CHECK((vtWidth % vtHeight == 0) && (vtWidth / vtHeight == vtHeight));
		cloudShapeNoise->initGLResource(vtHeight, vtHeight, vtWidth / vtHeight);
	}
	cloudErosionNoise = pathos::loadVolumeTextureFromTGA(CLOUD_EROSION_NOISE_FILE, "Texture_CloudErosionNoise");
	{
		uint32 vtWidth = cloudErosionNoise->getSourceImageWidth();
		uint32 vtHeight = cloudErosionNoise->getSourceImageHeight();
		CHECK((vtWidth % vtHeight == 0) && (vtWidth / vtHeight == vtHeight));
		cloudErosionNoise->initGLResource(vtHeight, vtHeight, vtWidth / vtHeight);
	}
}

void World_Game1::reloadScene()
{
	destroyAllActors();

	ActorBinder binder;
	binder.addBinding("SkyAtmosphere", &skyAtmosphere);
	binder.addBinding("Skybox", &skybox);
	binder.addBinding("SkyEquirectangularMap", &skyEquimap);
	binder.addBinding("Sun", &sun);
	binder.addBinding("PointLight0", &pointLight0);
	binder.addBinding("Sphere0", &sphere0);
	binder.addBinding("Landscape", &landscape);

	SceneLoader sceneLoader;
	sceneLoader.loadSceneDescription(this, SCENE_DESC_FILE, binder);

	// reloadScene() destroys all actors so respawn here :/
	playerController = spawnActor<PlayerController>();
	playerController->setPlayerPawn(sphere0);

	cloudscape = spawnActor<VolumetricCloudActor>();
	cloudscape->setTextures(weatherTexture, cloudShapeNoise, cloudErosionNoise);
	scene.cloud = cloudscape;

	setupScene();
}

void World_Game1::setupScene()
{
	sphere0->setStaticMesh(sphereMesh);
	landscape->setStaticMesh(landscapeMesh);
	landscape->getStaticMeshComponent()->castsShadow = false;
}
