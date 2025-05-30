#include "world_rc1.h"
#include "galaxy_gen.h"
#include "player_controller.h"
#include "lightning_effect.h"

#include "pathos/rhi/texture.h"
#include "pathos/render/light_probe_baker.h"

#include "pathos/mesh/static_mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/geometry_procedural.h"
#include "pathos/material/material.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/volumetric_cloud_actor.h"
#include "pathos/scene/sky_panorama_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/point_light_actor.h"

#include "pathos/loader/image_loader.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/spline_loader.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

#include "badger/math/random.h"
#include "badger/math/minmax.h"

// --------------------------------------------------------
// Constants

#define OBJ_GUARD_TOWER_FILE     "render_challenge_1/medieval_tower.obj"
#define OBJ_GUARD_TOWER_DIR      "render_challenge_1/"
#define OBJ_SPACESHIP_FILE       "render_challenge_1/spaceship.obj"
#define OBJ_SPACESHIP_DIR        "render_challenge_1/"

#define RING_ALBEDO              "resources/render_challenge_1/T_Brick.png"
#define RING_NORMAL              "resources/render_challenge_1/N_Brick.png"
#define RING_METALLIC            "resources/render_challenge_1/M_Brick.png"
#define RING_ROUGHNESS           "resources/render_challenge_1/R_Brick.png"
#define RING_LOCAL_AO            "resources/render_challenge_1/A_Brick.png"

#define SPLINE_1                 "render_challenge_1/SpaceshipCurve1.spline"
#define SPLINE_2                 "render_challenge_1/SpaceshipCurve2.spline"

#define CLOUD_WEATHER_MAP_FILE   "render_challenge_1/WeatherMap.png"
#define CLOUD_SHAPE_NOISE_FILE   "common/noiseShapePacked.tga"
#define CLOUD_EROSION_NOISE_FILE "common/noiseErosionPacked.tga"

#define NUM_LIGHTNING_PARTICLES  22

#define STARFIELD_WIDTH          4096
#define STARFIELD_HEIGHT         2048
#define STARFIELD_CUBEMAP_SIZE   512
#define STARFIELD_DUST_INTENSITY 10.0f

// #todo-cloud: I downscaled everything to 1/100 in https://github.com/codeonwort/pathosengine/pull/41,
// but volumetric clouds' min march step is tens of meters, so can't deal with 1 meter-sized occluders.
// Temporarily revert the scale of only world_rc1.
#define RC1_SCALE                100.0f

static const vector3             CAMERA_POSITION = RC1_SCALE * vector3(0.2f, 0.25f, 2.0f);
static const vector3             CAMERA_LOOK_AT  = RC1_SCALE * vector3(0.2f, 0.25f, 1.9f);
static const vector3             SUN_DIRECTION   = glm::normalize(vector3(0.0f, -1.0f, -1.0f));
static const vector3             SUN_COLOR       = vector3(1.0f, 1.0f, 1.0f);
static constexpr float           SUN_ILLUMINANCE = 1.0f; // #todo-cloud: Real world illuminance makes clouds too bright
static constexpr float           Y_OFFSET        = 5000.0f; // Offset every actor to match with cloud layer

// --------------------------------------------------------
// World

void World_RC1::onInitialize()
{
	// --------------------------------------------------------
	// Async load assets
	M_tower = Material::createMaterialInstance("guard_tower");
	AssetReferenceWavefrontOBJ assetRefGuardTower(OBJ_GUARD_TOWER_FILE, OBJ_GUARD_TOWER_DIR);
	assetRefGuardTower.addMaterialOverride("TowerMaterial", M_tower);
	gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRefGuardTower, this, &World_RC1::onLoadOBJ, 0);
	
	spaceship1 = spawnActor<SpaceshipActor>();
	spaceship1->setActorScale(RC1_SCALE * 0.3f);
	spaceship1->setActorLocation(RC1_SCALE * vector3(-3.47f, (Y_OFFSET / RC1_SCALE)- 10.98f, 16.48f));
	spaceship1->setActorRotation(Rotator(92.91f, 41.14f, 0.0f));
	{
		SplineLoader loader;
		std::string splineName;
		HermiteSpline spline1;
		CHECK(loader.load(SPLINE_1, true, splineName, spline1));
		spaceship1->setSpline(std::move(spline1));
	}
	spaceship2 = spawnActor<SpaceshipActor>();
	spaceship2->setActorScale(RC1_SCALE * 0.3f);
	spaceship2->setActorLocation(RC1_SCALE * vector3(12.57f, (Y_OFFSET / RC1_SCALE) - 10.98f, 3.48f));
	spaceship2->setActorRotation(Rotator(112.91f, -21.14f, 0.0f));
	{
		SplineLoader loader;
		std::string splineName;
		HermiteSpline spline2;
		CHECK(loader.load(SPLINE_2, true, splineName, spline2));
		spaceship2->setSpline(std::move(spline2));
	}

	// --------------------------------------------------------
	// Spawn actors
	playerController = spawnActor<PlayerController>();
	playerController->speedScale = RC1_SCALE;
	setupSky();
	setupScene();

	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
	getCamera().moveToPosition(RC1_SCALE * vector3(0.0f, (Y_OFFSET / RC1_SCALE) - 1.0f, 40.0f));

	// --------------------------------------------------------
	// Setup input
	ButtonBinding updateSky;
	updateSky.addInput(InputConstants::KEYBOARD_R);

	ButtonBinding traceCamera;
	traceCamera.addInput(InputConstants::KEYBOARD_C);

	InputManager* inputManager = getInputManager();
	inputManager->bindButtonPressed("updateSky", updateSky, [this]() {
		LOG(LogDebug, "Regenerate starfield");
		updateStarfield();
	});
	inputManager->bindButtonPressed("traceCamera", traceCamera, [this]() {
		vector3 position = getCamera().getPosition();
		Rotator rotation = Rotator(getCamera().getYaw(), getCamera().getPitch(), 0.0f); // no roll yet
		LOG(LogDebug, "Camera (x, y, z) = (%f, %f, %f), (pitch, yaw, roll) = (%f, %f, %f)",
			position.x, position.y, position.z, rotation.yaw, rotation.pitch, rotation.roll);
	});
}

void World_RC1::onTick(float deltaSeconds)
{
	vector3 ringRotations[6] = {
		vector3(25.0f, 12.0f, 25.0f),
		vector3(20.0f, -10.0f, 20.0f),
		vector3(-17.0f, 12.0f, 15.0f),
		vector3(14.0f, 17.0f, 7.0f),
		vector3(-5.0f, -8.0f, 4.0f),
		vector3(-3.0f, -2.5f, 2.0f),
	};
	for (uint32 i = 0; i < (uint32)rings.size(); ++i) {
		const auto& ring = rings[i];
		Rotator rot = ring->getActorRotation();
		rot.pitch += ringRotations[i].x * deltaSeconds;
		rot.yaw += ringRotations[i].y * deltaSeconds;
		rot.roll += ringRotations[i].z * deltaSeconds;
		ring->setActorRotation(rot);
	}

	auto& components = lightningSphere->getParticleComponents();
	for (uint32 i = 0; i < (uint32)components.size(); ++i) {
		components[i]->setRotation(rings[ringIndicesForParticleRotation[i]]->getActorRotation());
		components[i]->setLocation(vector3(0.0f, Y_OFFSET, 0.0f));
	}

	{
		HermiteSpline& spline  = spaceship1->getSpline();
		float totalTime        = spline.getTotalTime();
		float totalDistance    = spline.getTotalDistance();
		float worldTime        = gEngine->getWorldTime();

		float sampleDistance   = fmod(worldTime * 0.05f, 1.0f) * totalDistance;
		float sampleTime       = fmod(worldTime * 1.0f, totalTime);

		vector3 splineLocation = spline.locationAtDistance(sampleDistance);
		vector3 splineTangent  = glm::normalize(spline.tangentAtDistance(sampleDistance));
		//vector3 splineLocation = spline.locationAtTime(sampleTime);
		//vector3 splineTangent  = glm::normalize(spline.tangentAtTime(sampleTime));

		ModelTransform transform(
			RC1_SCALE * vector3(-3.47f, (Y_OFFSET / RC1_SCALE) - 10.98f, 16.48f),
			Rotator(),
			RC1_SCALE * vector3(10.0f));
		splineLocation = vector3(transform.getMatrix() * vector4(splineLocation, 1.0f));

		spaceship1->setActorLocation(splineLocation);
		spaceship1->setActorRotation(Rotator::directionToYawPitch(splineTangent));

		float deltaDist        = 1.0f;
		float cameraSampleDist = sampleDistance < deltaDist ? sampleDistance - deltaDist + totalDistance : sampleDistance - deltaDist;
		vector3 cameraPos      = spline.locationAtDistance(cameraSampleDist);
		cameraPos              = vector3(transform.getMatrix() * vector4(cameraPos, 1.0f));
		// Uncomment one line to auto-pilot the camera
		//getCamera().lookAt(cameraPos, splineLocation, vector3(0.0f, 1.0f, 0.0f)); // Force position and rotation
		//getCamera().moveToPosition(cameraPos);                                    // Force position only
	}
	{
		HermiteSpline& spline  = spaceship2->getSpline();
		float totalTime        = spline.getTotalTime();
		float totalDistance    = spline.getTotalDistance();
		float worldTime        = gEngine->getWorldTime();

		float sampleDistance   = fmod(worldTime * 0.05f, 1.0f) * totalDistance;
		float sampleTime       = fmod(worldTime * 1.0f, totalTime);

		vector3 splineLocation = spline.locationAtDistance(sampleDistance);
		vector3 splineTangent  = glm::normalize(spline.tangentAtDistance(sampleDistance));
		//vector3 splineLocation = spline.locationAtTime(sampleTime);
		//vector3 splineTangent  = glm::normalize(spline.tangentAtTime(sampleTime));

		ModelTransform transform(
			RC1_SCALE * vector3(-23.47f, (Y_OFFSET / RC1_SCALE) - 10.98f, 16.48f),
			Rotator(),
			RC1_SCALE * vector3(10.0f));
		splineLocation = vector3(transform.getMatrix() * vector4(splineLocation, 1.0f));

		spaceship2->setActorLocation(splineLocation);
		spaceship2->setActorRotation(Rotator::directionToYawPitch(splineTangent));
	}
}

void World_RC1::setupSky()
{
	{
		TextureCreateParams createParams;
		createParams.width           = STARFIELD_WIDTH;
		createParams.height          = STARFIELD_HEIGHT;
		createParams.depth           = 1;
		createParams.mipLevels       = 1;
		createParams.glDimension     = GL_TEXTURE_2D;
		createParams.glStorageFormat = GL_RGBA16F;
		createParams.debugName       = "Texture_Starfield";
		starfieldTexture = new Texture(createParams);
		starfieldTexture->createGPUResource();
	}
	GalaxyGenerator::renderStarField(starfieldTexture, STARFIELD_WIDTH, STARFIELD_HEIGHT, STARFIELD_DUST_INTENSITY);

	panoramaSky = spawnActor<PanoramaSkyActor>();
	panoramaSky->setTexture(starfieldTexture);

	// Volumetric cloud
	auto calcVolumeSize = [](const ImageBlob* imageBlob) -> vector3ui {
		uint32 vtWidth = imageBlob->width;
		uint32 vtHeight = imageBlob->height;
		CHECK((vtWidth % vtHeight == 0) && (vtWidth / vtHeight == vtHeight));
		return vector3ui(vtHeight, vtHeight, vtWidth / vtHeight);
	};
	ImageBlob* weatherMapBlob = ImageUtils::loadImage(CLOUD_WEATHER_MAP_FILE);
	ImageBlob* cloudShapeNoiseBlob = ImageUtils::loadImage(CLOUD_SHAPE_NOISE_FILE);
	ImageBlob* cloudErosionNoiseBlob = ImageUtils::loadImage(CLOUD_EROSION_NOISE_FILE);
	Texture* weatherTexture = ImageUtils::createTexture2DFromImage(weatherMapBlob, 1, false, true, "Texture_WeatherMap");
	Texture* cloudShapeNoise = ImageUtils::createTexture3DFromImage(cloudShapeNoiseBlob, calcVolumeSize(cloudShapeNoiseBlob), 0, false, true, "Texture_CloudShapeNoise");
	Texture* cloudErosionNoise = ImageUtils::createTexture3DFromImage(cloudErosionNoiseBlob, calcVolumeSize(cloudErosionNoiseBlob), 0, false, true, "Texture_CloudErosionNoise");
	auto cloudscape = spawnActor<VolumetricCloudActor>();
	cloudscape->setTextures(weatherTexture, cloudShapeNoise, cloudErosionNoise);
}

void World_RC1::setupScene()
{
	//////////////////////////////////////////////////////////////////////////
	// Light
	auto dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setDirection(SUN_DIRECTION);
	dirLight->setColorAndIlluminance(SUN_COLOR, SUN_ILLUMINANCE);

	auto pointLight = spawnActor<PointLightActor>();
	pointLight->setActorLocation(0.0f, Y_OFFSET, 0.0f);
	pointLight->setColor(vector3(1.0f, 1.0f, 1.0f));
	pointLight->setIntensity(RC1_SCALE * RC1_SCALE * 1000.0f);
	pointLight->setAttenuationRadius(RC1_SCALE * 50.0f);
	pointLight->setSourceRadius(RC1_SCALE * 0.4f);

	//////////////////////////////////////////////////////////////////////////
	// Materials

	assetPtr<Material> material_ring = Material::createMaterialInstance("pbr_texture");
	{
		constexpr uint32 mipLevels = 0;
		constexpr bool sRGB = true;
		Texture* albedo = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(RING_ALBEDO), mipLevels, sRGB);
		Texture* normal = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(RING_NORMAL), mipLevels, !sRGB);
		Texture* metallic = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(RING_METALLIC), mipLevels, !sRGB);
		Texture* roughness = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(RING_ROUGHNESS), mipLevels, !sRGB);
		Texture* localAO = ImageUtils::createTexture2DFromImage(ImageUtils::loadImage(RING_LOCAL_AO), mipLevels, !sRGB);
		material_ring->setTextureParameter("albedo", albedo);
		material_ring->setTextureParameter("normal", normal);
		material_ring->setTextureParameter("metallic", metallic);
		material_ring->setTextureParameter("roughness", roughness);
		material_ring->setTextureParameter("localAO", localAO);
	}

	//////////////////////////////////////////////////////////////////////////
	// Objects

	lightningSphere = spawnActor<LightningActor>();
	lightningSphere->setActorScale(RC1_SCALE * 0.4f);
	lightningSphere->setActorLocation(pointLight->getActorLocation());

	constexpr uint32 numRings  = 6;
	const float ring_gap       = RC1_SCALE * 0.4f;
	const float ring_width     = RC1_SCALE * 1.0f;
	const float ring_thickness = RC1_SCALE * 0.5f;

	float innerRadius = RC1_SCALE * 1.5f;
	float outerRadius = innerRadius + ring_width;
	std::vector<std::vector<float>> ringSegRanges = {
		{0.0f, 280.0f},
		{30.0f, 120.0f, 140.0f, 310.0f},
		{150.0f, 260.0f},
		{50.0f, 180.0f, 250.0f, 300.0f},
		{0.0f, 100.0f, 120.0f, 200.0f, 220.0f, 320.0f},
		{0.0f, 70.0f, 90.0f, 260.0f},
	};
	for (uint32 i = 0; i < numRings; ++i) {
		auto ring = spawnActor<RingActor>();
		ring->buildRing(innerRadius, outerRadius, ring_thickness + (i * RC1_SCALE * 0.2f), ringSegRanges[i]);
		innerRadius = outerRadius + ring_gap;
		outerRadius = innerRadius + (ring_width + i * RC1_SCALE * 0.55f);
		rings.push_back(ring);

		ring->setActorLocation(0.0f, Y_OFFSET, 0.0f);

		// #todo-lod
		const uint32 LOD = 0;
		ring->getStaticMesh()->getLOD(LOD).setMaterial(0, material_ring);
	}

	for (uint32 i = 0; i < NUM_LIGHTNING_PARTICLES; ++i) {
		// Select outer rings more frequently
		uint32 ringIx = badger::clamp(0u, (uint32)(numRings * Random()), numRings - 1);
		const auto& ring = rings[ringIx];
		lightningSphere->generateParticle(vector3(0.0f), ring->getRandomInnerPosition(), RC1_SCALE);
		ringIndicesForParticleRotation.push_back(ringIx);
	}
}

void World_RC1::updateStarfield()
{
	gEngine->executeConsoleCommand("recompile_shaders");
	GalaxyGenerator::renderStarField(starfieldTexture, STARFIELD_WIDTH, STARFIELD_HEIGHT, STARFIELD_DUST_INTENSITY);
	panoramaSky->setTexture(starfieldTexture);
}

void World_RC1::onLoadOBJ(OBJLoader* loader, uint64 payload)
{
	LOG(LogInfo, "Load guard tower model");

	guardTower = spawnActor<StaticMeshActor>();
	guardTower->setStaticMesh(loader->craftMeshFromAllShapes());
	guardTower->setActorScale(RC1_SCALE * 10.0f);
	guardTower->setActorLocation(RC1_SCALE * vector3(0.0f, (Y_OFFSET / RC1_SCALE) - 47.0f, 0.0f));

	M_tower->setTextureParameter("albedo", loader->findTexture("T_Tower.png"));
	M_tower->setTextureParameter("normal", loader->findTexture("N_Tower.png"));
	M_tower->setTextureParameter("roughness", loader->findTexture("R_Tower.png"));
	M_tower->setTextureParameter("metallic", loader->findTexture("M_Tower.png"));
}

// --------------------------------------------------------
// RingActor

RingActor::RingActor()
{
	G = makeAssetPtr<ProceduralGeometry>();
	
	// Overriden later by World_RC1::setupScene().
	M = Material::createMaterialInstance("solid_color");
	M->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	M->setConstantParameter("metallic", 0.0f);
	M->setConstantParameter("roughness", 1.0f);
	M->setConstantParameter("emissive", vector3(0.0f, 0.0f, 0.0f));

	setStaticMesh(makeAssetPtr<StaticMesh>(G, M));
}

void RingActor::buildRing(
	float innerRadius,
	float outerRadius,
	float thickness,
	const std::vector<float>& segmentRanges)
{
	G->clear();
	innerVertexIndices.clear();

	static const float defaultRange[] = { 0.0f, 360.0f };
	const float* segRanges = segmentRanges.size() == 0 ? defaultRange : segmentRanges.data();
	const uint32 numSegments = segmentRanges.size() == 0 ? 1 : (uint32)segmentRanges.size() / 2;

	constexpr uint32 numSubdivisions = 60;
	constexpr auto n = numSubdivisions;
	std::vector<vector3> positions(numSegments * (n * 4 + 8));
	std::vector<vector2> uvs(numSegments * (n * 4 + 8));
	std::vector<uint32> indices;
	indices.reserve(numSegments * (n * 24 + 12));
	innerVertexIndices.reserve(numSegments * (n * 4) / 2);

	uint32 i0 = 0;

	for (uint32 segmentIndex = 0; segmentIndex < numSegments; ++segmentIndex) {
		const float dz = thickness * 0.5f;
		const float startAngle = glm::radians(segRanges[segmentIndex * 2]);
		const float endAngle = glm::radians(segRanges[segmentIndex * 2 + 1]);

		// clockwise
		auto makeQuad = [&indices](uint32 a, uint32 b, uint32 c, uint32 d) {
			indices.push_back(a); indices.push_back(b); indices.push_back(d);
			indices.push_back(b); indices.push_back(c); indices.push_back(d);
		};

		for (uint32 i = i0; i < i0 + n; ++i) {
			const float ratio = (float)(i - i0) / (n - 1);
			float angle = startAngle + (endAngle - startAngle) * ratio;
			float cosAngle = cosf(angle);
			float sinAngle = sinf(angle);

			innerVertexIndices.push_back(i);
			innerVertexIndices.push_back(i + 2 * n);

			positions[i + 0 * n] = vector3(innerRadius * cosAngle, innerRadius * sinAngle, +dz);
			positions[i + 1 * n] = vector3(outerRadius * cosAngle, outerRadius * sinAngle, +dz);
			positions[i + 2 * n] = vector3(innerRadius * cosAngle, innerRadius * sinAngle, -dz);
			positions[i + 3 * n] = vector3(outerRadius * cosAngle, outerRadius * sinAngle, -dz);

			uvs[i + 0 * n] = vector2(ratio, 0.0f);
			uvs[i + 1 * n] = vector2(ratio, 0.25f);
			uvs[i + 2 * n] = vector2(ratio, 0.5f);
			uvs[i + 3 * n] = vector2(ratio, 1.0f);

			if (i - i0 != n - 1) {
				makeQuad(i, i + n, i + n + 1, i + 1);                                 // front
				makeQuad(2 * n + i, 2 * n + i + 1, 2 * n + i + n + 1, 2 * n + i + n); // back
				makeQuad(i, i + 1, i + 2 * n + 1, i + 2 * n);                         // inner
				makeQuad(i + n, 2 * n + i + n, 2 * n + i + n + 1, i + n + 1);         // outer
			} else {
				//makeQuad(i, i + n, i + 1, i + 1 - n);
				//makeQuad(2 * n + i, 2 * n + i + 1 - n, 2 * n + i + 1, 2 * n + i + n);
				//makeQuad(i, i + 1 - n, i + 1 + n, i + 2 * n);
				//makeQuad(i + n, i + 3 * n, i + 2 * n + 1, i + 1);
			}
		}

		i0 += n * 4;

		float angle = startAngle;
		float cosAngle = cosf(angle);
		float sinAngle = sinf(angle);
		positions[i0 + 0] = vector3(innerRadius * cosAngle, innerRadius * sinAngle, +dz);
		positions[i0 + 1] = vector3(outerRadius * cosAngle, outerRadius * sinAngle, +dz);
		positions[i0 + 2] = vector3(innerRadius * cosAngle, innerRadius * sinAngle, -dz);
		positions[i0 + 3] = vector3(outerRadius * cosAngle, outerRadius * sinAngle, -dz);
		uvs[i0 + 0] = vector2(0.0f, 0.0f);
		uvs[i0 + 1] = vector2(1.0f, 0.0f);
		uvs[i0 + 2] = vector2(0.0f, 1.0f);
		uvs[i0 + 3] = vector2(1.0f, 1.0f);
		makeQuad(i0, i0 + 2, i0 + 3, i0 + 1);

		i0 += 4;

		angle = endAngle;
		cosAngle = cosf(angle);
		sinAngle = sinf(angle);
		positions[i0 + 0] = vector3(innerRadius * cosAngle, innerRadius * sinAngle, +dz);
		positions[i0 + 1] = vector3(outerRadius * cosAngle, outerRadius * sinAngle, +dz);
		positions[i0 + 2] = vector3(innerRadius * cosAngle, innerRadius * sinAngle, -dz);
		positions[i0 + 3] = vector3(outerRadius * cosAngle, outerRadius * sinAngle, -dz);
		uvs[i0 + 0] = vector2(0.0f, 0.0f);
		uvs[i0 + 1] = vector2(1.0f, 0.0f);
		uvs[i0 + 2] = vector2(0.0f, 1.0f);
		uvs[i0 + 3] = vector2(1.0f, 1.0f);
		makeQuad(i0 + 1, i0 + 3, i0 + 2, i0 + 0);

		i0 += 4;
	}

	G->updatePositionData((float*)positions.data(), (uint32)(positions.size() * 3));
	G->updateUVData((float*)uvs.data(), (uint32)(uvs.size() * 2));
	G->updateIndexData(indices.data(), (uint32)indices.size());
	G->calculateNormals();
	G->calculateTangentBasis();
}

vector3 RingActor::getRandomInnerPosition() const
{
	uint32 ix = (uint32)(innerVertexIndices.size() * Random());
	return G->getPosition(innerVertexIndices[ix]);
}

// --------------------------------------------------------
// SpaceshipActor

void SpaceshipActor::onSpawn()
{
	AssetReferenceWavefrontOBJ assetRef(OBJ_SPACESHIP_FILE, OBJ_SPACESHIP_DIR);
	gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRef, this, &SpaceshipActor::onLoadOBJ, 0);
}

void SpaceshipActor::onLoadOBJ(OBJLoader* loader, uint64 payload)
{
	LOG(LogInfo, "Load spaceship model");

	setStaticMesh(loader->craftMeshFromAllShapes());
}
