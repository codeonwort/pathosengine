#include "world_racing_game.h"
#include "player_controller.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/landscape_actor.h"
#include "pathos/scene/landscape_component.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/skybox_actor.h"
#include "pathos/scene/sky_panorama_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/volumetric_cloud_actor.h"
#include "pathos/loader/scene_loader.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/input/input_manager.h"

#include "badger/math/random.h"

#define SCENE_DESC_FILE          "resources/racing_game/test_scene.json"
#define LANDSCAPE_ALBEDO_MAP     "resources/racing_game/everest_albedo.png"
#define LANDSCAPE_HEIGHT_MAP     "resources/racing_game/everest_height.png"

// #wip: Proper weather map for volumetric clouds
//#define CLOUD_WEATHER_MAP_FILE   "resources/racing_game/WeatherMap.png"
#define CLOUD_WEATHER_MAP_FILE   "resources/render_challenge_1/WeatherMap.png"
#define CLOUD_SHAPE_NOISE_FILE   "resources/common/noiseShapePacked.tga"
#define CLOUD_EROSION_NOISE_FILE "resources/common/noiseErosionPacked.tga"

// Doesn't matter as the player controller will control the camera.
#define CAMERA_POSITION          vector3(0.0f, 0.0f, 0.5f)
#define CAMERA_LOOK_AT           vector3(0.0f, 0.0f, 0.0f)

#define PLAYERCAM_HEIGHT_OFFSET  2.8f
#define PLAYERCAM_FORWARD_OFFSET 10.0f

#define NUM_TREES                100

const std::vector<AssetReferenceWavefrontOBJ> wavefrontModelRefs = {
	{
		"resources_external/SportsCar/sportsCar.obj",
		"resources_external/SportsCar/",
	},
	{
		"resources_external/ScrubPineTree/scrubPine.obj",
		"resources_external/ScrubPineTree/",
	},
};

void World_RacingGame::onInitialize() {
	SCOPED_CPU_COUNTER(World_Game1_initialize);

	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));

	prepareAssets();
	reloadScene();

	gEngine->registerConsoleCommand("reload_scene", [this](const std::string& command) {
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

void World_RacingGame::onDestroy() {
	//
}

void World_RacingGame::onTick(float deltaSeconds) {
	//
}

void World_RacingGame::prepareAssets() {
	for (size_t i = 0u; i < wavefrontModelRefs.size(); ++i) {
		gEngine->getAssetStreamer()->enqueueWavefrontOBJ(wavefrontModelRefs[i], this, &World_RacingGame::onLoadOBJ, i);
	}

	Material* M_color = Material::createMaterialInstance("solid_color");
	M_color->setConstantParameter("albedo", vector3(0.9f, 0.1f, 0.1f));
	M_color->setConstantParameter("metallic", 0.0f);
	M_color->setConstantParameter("roughness", 0.2f);
	M_color->setConstantParameter("emissive", vector3(0.0f));

	auto G_sphere = new SphereGeometry(1.0f, 30);

	carDummyMesh = makeShared<Mesh>(G_sphere, M_color);

	// Volumetric Clouds
	auto calcVolumeSize = [](const ImageBlob* imageBlob) -> vector3ui {
		uint32 vtWidth = imageBlob->width;
		uint32 vtHeight = imageBlob->height;
		CHECK((vtWidth % vtHeight == 0) && (vtWidth / vtHeight == vtHeight));
		return vector3ui(vtHeight, vtHeight, vtWidth / vtHeight);
	};
	ImageBlob* weatherMapBlob = ImageUtils::loadImage(CLOUD_WEATHER_MAP_FILE);
	ImageBlob* cloudShapeNoiseBlob = ImageUtils::loadImage(CLOUD_SHAPE_NOISE_FILE);
	ImageBlob* cloudErosionNoiseBlob = ImageUtils::loadImage(CLOUD_EROSION_NOISE_FILE);
	weatherTexture = ImageUtils::createTexture2DFromImage(weatherMapBlob, 1, false, true, "Texture_WeatherMap");
	cloudShapeNoise = ImageUtils::createTexture3DFromImage(cloudShapeNoiseBlob, calcVolumeSize(cloudShapeNoiseBlob), 0, false, true, "Texture_CloudShapeNoise");
	cloudErosionNoise = ImageUtils::createTexture3DFromImage(cloudErosionNoiseBlob, calcVolumeSize(cloudErosionNoiseBlob), 0, false, true, "Texture_CloudErosionNoise");
	gConsole->addLine(L"r.cloud.minY 1000", true, false);
	gConsole->addLine(L"r.cloud.maxY 4000", true, false);
	gConsole->addLine(L"r.cloud.sunIntensityScale 80", true, false);
}

void World_RacingGame::reloadScene() {
	destroyAllActors();

	ActorBinder binder;
	binder.addBinding("SkyAtmosphere", &skyAtmosphere);
	binder.addBinding("Skybox", &skybox);
	binder.addBinding("SkyEquirectangularMap", &skyEquimap);
	binder.addBinding("Sun", &sun); // #todo-cloud: Real world illuminance makes clouds too bright
	binder.addBinding("PointLight0", &pointLight0);
	binder.addBinding("PlayerCar", &playerCar);
	binder.addBinding("Landscape", &landscape);

	SceneLoader sceneLoader;
	sceneLoader.loadSceneDescription(this, SCENE_DESC_FILE, binder);

	skybox->getSkyComponent()->setVisibility(false);
	//skyEquimap->getSkyComponent()->setVisibility(false);
	skyAtmosphere->getSkyComponent()->setVisibility(false);

	// reloadScene() destroys all actors so respawn here :/
	playerController = spawnActor<PlayerController>();
	playerController->setPlayerPawn(playerCar);
	playerController->cameraHeightOffset = PLAYERCAM_HEIGHT_OFFSET;
	playerController->cameraForwardOffset = PLAYERCAM_FORWARD_OFFSET;

	cloudscape = spawnActor<VolumetricCloudActor>();
	cloudscape->setTextures(weatherTexture, cloudShapeNoise, cloudErosionNoise);

	treeActors.clear();
	for (uint32 i = 0; i < NUM_TREES; ++i) {
		StaticMeshActor* tree = spawnActor<StaticMeshActor>();
		float x = (Random() + 0.02f) * 500.0f;
		float z = (Random() + 0.02f) * 500.0f;
		if (Random() < 0.5f) x *= -1;
		if (Random() < 0.5f) z *= -1;
		tree->setActorLocation(x, -2.0f, z);
		tree->setActorScale(0.05f);
		treeActors.push_back(tree);
	}

	// #wip-landscape: Depth prepass does not handle landscape proxy yet.
	gConsole->addLine(L"r.depth_prepass 0");

	Texture* albedoTexture = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(LANDSCAPE_ALBEDO_MAP), 0, true, true, "Texture_Landscape_Albedo");
	Texture* heightTexture = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(LANDSCAPE_HEIGHT_MAP), 0, false, true, "Texture_Landscape_Height");

	// #wip-landscape: Temp material
	Material* M_landscape = Material::createMaterialInstance("landscape");
	M_landscape->setTextureParameter("albedo", albedoTexture);
	M_landscape->setTextureParameter("heightmap", heightTexture);

	landscape->getLandscapeComponent()->setMaterial(M_landscape);
	landscape->initializeSectors(80.0f, 80.0f, 10, 10);
	landscape->setActorLocation(-400.0f, -30.0f, -400.0f);

	setupScene();
}

void World_RacingGame::setupScene() {
	playerCar->setStaticMesh(carMesh ? carMesh.get() : carDummyMesh.get());
	for (size_t i = 0; i < treeActors.size(); ++i) {
		treeActors[i]->setStaticMesh(treeMesh.get());
	}
}

void World_RacingGame::onLoadOBJ(OBJLoader* loader, uint64 payload) {
	uint32 assetIndex = (uint32)payload;

	if (!loader->isValid()) {
		LOG(LogError, "Failed to load a wavefront model: %s", wavefrontModelRefs[assetIndex].filepath.c_str());
		return;
	}

	if (assetIndex == 0) {
		carMesh = sharedPtr<Mesh>(loader->craftMeshFromAllShapes(true));
		setupScene();
	} else if (assetIndex == 1) {
		treeMesh = sharedPtr<Mesh>(loader->craftMeshFromAllShapes());
		treeMesh->doubleSided = true;
		setupScene();
	}
}
