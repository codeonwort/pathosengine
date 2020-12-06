#include "world_rc1.h"
#include "galaxy_gen.h"
#include "player_controller.h"
#include "lightning_effect.h"

#include "badger/math/random.h"

#include "pathos/render/sky_ansel.h"
#include "pathos/render/sky_clouds.h"
#include "pathos/loader/imageloader.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/spline_loader.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/geometry_procedural.h"
#include "pathos/texture/volume_texture.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/light/point_light_actor.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/log.h"

#define OBJ_GUARD_TOWER_FILE     "render_challenge_1/medieval_tower.obj"
#define OBJ_GUARD_TOWER_DIR      "render_challenge_1/"
#define OBJ_SPACESHIP_FILE       "render_challenge_1/spaceship.obj"
#define OBJ_SPACESHIP_DIR        "render_challenge_1/"

#define CLOUD_WEATHER_MAP_FILE   "render_challenge_1/WeatherMap.png"
#define CLOUD_SHAPE_NOISE_FILE   "render_challenge_1/noiseShape.tga"
#define CLOUD_EROSION_NOISE_FILE "render_challenge_1/noiseErosion.tga"

const vector3       SUN_DIRECTION        = glm::normalize(vector3(0.0f, -1.0f, 0.0f));
const vector3       SUN_RADIANCE         = 1.0f * vector3(1.0f, 1.0f, 1.0f);
constexpr float     Y_OFFSET             = 6500.0f; // Offset every actor to match with cloud layer

void World_RC1::onInitialize()
{
	//////////////////////////////////////////////////////////////////////////
	// Async load assets
	AssetReferenceWavefrontOBJ assetRefGuardTower(OBJ_GUARD_TOWER_FILE, OBJ_GUARD_TOWER_DIR);
	gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRefGuardTower, this, &World_RC1::onLoadOBJ);
	
	spaceship1 = spawnActor<SpaceshipActor>();
	spaceship1->setActorScale(30.0f);
	spaceship1->setActorLocation(vector3(-347.0f, Y_OFFSET - 1098.0f, 1648.0f));
	spaceship1->setActorRotation(Rotator(92.91f, 41.14f, 0.0f));

	SplineLoader loader;
	std::string splineName;
	HermiteSpline spline1;
	CHECK(loader.load("render_challenge_1/spaceship1.spline", true, splineName, spline1));
	spaceship1->setSpline(std::move(spline1));

	spaceship2 = spawnActor<SpaceshipActor>();
	spaceship2->setActorScale(30.0f);
	spaceship2->setActorLocation(vector3(1257.0f, Y_OFFSET - 1098.0f, 348.0f));
	spaceship2->setActorRotation(Rotator(112.91f, -21.14f, 0.0f));

	//////////////////////////////////////////////////////////////////////////
	// Spawn actors
	playerController = spawnActor<PlayerController>();
	setupSky();
	setupScene();

	getCamera().moveToPosition(0.0f, Y_OFFSET - 100.0f, 4000.0f);

	//////////////////////////////////////////////////////////////////////////
	// Setup input
	ButtonBinding updateSky;
	updateSky.addInput(InputConstants::KEYBOARD_R);

	ButtonBinding traceCamera;
	traceCamera.addInput(InputConstants::KEYBOARD_C);

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindButtonPressed("updateSky", updateSky, [this]()
		{
			updateStarfield();
		}
	);
	inputManager->bindButtonPressed("traceCamera", traceCamera, [this]()
		{
			vector3 position = getCamera().getPosition();
			Rotator rotation = Rotator(getCamera().getYaw(), getCamera().getPitch(), 0.0f); // no roll yet
			LOG(LogDebug, "Camera (x, y, z) = (%f, %f, %f), (pitch, yaw, roll) = (%f, %f, %f)",
				position.x, position.y, position.z, rotation.yaw, rotation.pitch, rotation.roll);
		}
	);
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
		RingActor* ring = rings[i];
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
		vector3 splineTangent = glm::normalize(spline.tangentAtDistance(sampleDistance));
		//vector3 splineLocation = spline.locationAtTime(sampleTime);
		//vector3 splineTangent  = glm::normalize(spline.tangentAtTime(sampleTime));

		ModelTransform transform(vector3(-347.0f, Y_OFFSET - 1098.0f, 1648.0f), Rotator(), vector3(1000.0f));
		splineLocation = vector3(transform.getMatrix() * vector4(splineLocation, 1.0f));

		spaceship1->setActorLocation(splineLocation);
		spaceship1->setActorRotation(Rotator::directionToYawPitch(splineTangent));
	}
}

void World_RC1::setupSky()
{
	GalaxyGenerator::createStarField(starfield, 2048, 1024);
	glObjectLabel(GL_TEXTURE, starfield, -1, "Texture: Starfield");

	GLuint cubemapForIBL = IrradianceBaker::bakeCubemap(starfield, 512);
	glObjectLabel(GL_TEXTURE, cubemapForIBL, -1, "Texture IBL: cubemapForIBL");

	// diffuse irradiance
	GLuint irradianceMap = IrradianceBaker::bakeIrradianceMap(cubemapForIBL, 32, false);
	glObjectLabel(GL_TEXTURE, irradianceMap, -1, "Texture IBL: diffuse irradiance");
	scene.irradianceMap = irradianceMap;

	// specular IBL
	GLuint prefilteredEnvMap;
	uint32 mipLevels;
	IrradianceBaker::bakePrefilteredEnvMap(cubemapForIBL, 128, prefilteredEnvMap, mipLevels);
	glObjectLabel(GL_TEXTURE, prefilteredEnvMap, -1, "Texture IBL: specular IBL (prefiltered env map)");

	scene.prefilterEnvMap = prefilteredEnvMap;
	scene.prefilterEnvMapMipLevels = mipLevels;

	scene.sky = new AnselSkyRendering(starfield);

	// Volumetric cloud
	{
		GLuint weatherTexture = pathos::createTextureFromBitmap(pathos::loadImage(CLOUD_WEATHER_MAP_FILE), true, false);
		glObjectLabel(GL_TEXTURE, weatherTexture, -1, "Texture: WeatherMap");
		VolumeTexture* cloudShapeNoise = pathos::loadVolumeTextureFromTGA(CLOUD_SHAPE_NOISE_FILE, "Texture_CloudShapeNoise");
		{
			uint32 vtWidth = cloudShapeNoise->getSourceImageWidth();
			uint32 vtHeight = cloudShapeNoise->getSourceImageHeight();
			CHECK((vtWidth % vtHeight == 0) && (vtWidth / vtHeight == vtHeight));
			cloudShapeNoise->initGLResource(vtHeight, vtHeight, vtWidth / vtHeight);
		}
		VolumeTexture* cloudErosionNoise = pathos::loadVolumeTextureFromTGA(CLOUD_EROSION_NOISE_FILE, "Texture_CloudErosionNoise");
		{
			uint32 vtWidth = cloudErosionNoise->getSourceImageWidth();
			uint32 vtHeight = cloudErosionNoise->getSourceImageHeight();
			CHECK((vtWidth % vtHeight == 0) && (vtWidth / vtHeight == vtHeight));
			cloudErosionNoise->initGLResource(vtHeight, vtHeight, vtWidth / vtHeight);
		}
		scene.cloud = spawnActor<VolumetricCloudActor>();
		scene.cloud->setTextures(weatherTexture, cloudShapeNoise, cloudErosionNoise);
	}
}

void World_RC1::setupScene()
{
	//////////////////////////////////////////////////////////////////////////
	// Light
	DirectionalLightActor* dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setLightParameters(SUN_DIRECTION, SUN_RADIANCE);

	PointLightActor* pLight = spawnActor<PointLightActor>();
	pLight->setActorLocation(0.0f, Y_OFFSET, 0.0f);
	pLight->setLightParameters(5000.0f * vector3(1.0f, 1.0f, 1.0f), 10000.0f);

	//////////////////////////////////////////////////////////////////////////
	auto geom_sphere = new SphereGeometry(5.0f, 30);
	geom_sphere->calculateTangentBasis();

	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(2.0f, 0.2f, 0.2f);
		color->setMetallic(0.2f);
		color->setRoughness(0.1f);
	}

	PBRTextureMaterial* material_pbr;
	{
		constexpr bool genMipmap = true;
		constexpr bool sRGB = true;
		GLuint albedo = pathos::createTextureFromBitmap(loadImage("resources/render_challenge_1/T_Brick.png"), genMipmap, sRGB);
		GLuint normal = pathos::createTextureFromBitmap(loadImage("resources/render_challenge_1/N_Brick.png"), genMipmap, !sRGB);
		GLuint metallic = pathos::createTextureFromBitmap(loadImage("resources/render_challenge_1/M_Brick.png"), genMipmap, !sRGB);
		GLuint roughness = pathos::createTextureFromBitmap(loadImage("resources/render_challenge_1/R_Brick.png"), genMipmap, !sRGB);
		GLuint ao = pathos::createTextureFromBitmap(loadImage("resources/render_challenge_1/A_Brick.png"), genMipmap, !sRGB);
		material_pbr = new PBRTextureMaterial(albedo, normal, metallic, roughness, ao);
	}

	//////////////////////////////////////////////////////////////////////////
	// Objects

	lightningSphere = spawnActor<LightningActor>();
	lightningSphere->setActorScale(40.0f);
	lightningSphere->setActorLocation(0.0f, Y_OFFSET, 0.0f);

	constexpr uint32 numRings = 6;
	const float ring_gap = 40.0f;
	const float ring_width = 100.0f;
	const float ring_thickness = 50.0f;
	float innerRadius = 150.0f;
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
		ring->buildRing(innerRadius, outerRadius, ring_thickness + (i * 20.0f), ringSegRanges[i]);
		innerRadius = outerRadius + ring_gap;
		outerRadius = innerRadius + (ring_width + i * 50.0f);
		rings.push_back(ring);

		ring->setActorLocation(0.0f, Y_OFFSET, 0.0f);
		ring->getStaticMesh()->setMaterial(0, material_pbr);
	}

	const uint32 numParticles = 10;
	for (uint32 i = 0; i < numParticles; ++i) {
		uint32 ringIx = (uint32)(numRings * Random());
		RingActor* ring = rings[ringIx];
		lightningSphere->generateParticle(vector3(0.0f), ring->getRandomInnerPosition());
		ringIndicesForParticleRotation.push_back(ringIx);
	}
}

void World_RC1::updateStarfield()
{
	gEngine->execute("recompile_shaders");
	GalaxyGenerator::createStarField(starfield, 2048, 1024);
}

void World_RC1::onLoadOBJ(OBJLoader* loader)
{
	LOG(LogInfo, "Load guard tower model");

	guardTower = spawnActor<StaticMeshActor>();
	guardTower->setStaticMesh(loader->craftMeshFromAllShapes());
	guardTower->setActorScale(1000.0f);
	guardTower->setActorLocation(vector3(0.0f, Y_OFFSET - 4700.0f, 0.0f));

	// #todo-material: hack
	static_cast<PBRTextureMaterial*>(guardTower->getStaticMesh()->getMaterials()[0])->useTriplanarMapping = true;
}

//////////////////////////////////////////////////////////////////////////

RingActor::RingActor()
{
	G = new ProceduralGeometry;
	
	M = new ColorMaterial;
	M->setAlbedo(1.8f, 1.8f, 1.8f);
	M->setRoughness(1.0f);
	M->setMetallic(0.0f);

	setStaticMesh(new Mesh(G, M));
}

void RingActor::buildRing(float innerRadius, float outerRadius, float thickness, const std::vector<float>& segmentRanges)
{
	G->clear();
	innerVertexIndices.clear();

	static const float defaultRange[] = { 0.0f, 360.0f };
	const float* segRanges = segmentRanges.size() == 0 ? defaultRange : segmentRanges.data();
	const uint32 numSegments = segmentRanges.size() == 0 ? 1 : (uint32)segmentRanges.size() / 2;

	constexpr uint32 numSubdivisions = 60;
	constexpr auto n = numSubdivisions;
	std::vector<vector3> positions(numSegments * n * 4);
	std::vector<vector2> uvs(numSegments * n * 4);
	std::vector<uint32> indices;
	indices.reserve(numSegments * n * 24);
	innerVertexIndices.reserve(positions.size() / 2);

	uint32 i0 = 0;

	for(uint32 segmentIndex = 0; segmentIndex < numSegments; ++segmentIndex) {
		const float dz = thickness * 0.5f;
		const float startAngle = glm::radians(segRanges[segmentIndex * 2]);
		const float endAngle = glm::radians(segRanges[segmentIndex * 2 + 1]);

		// clockwise
		auto makeQuad = [&indices](uint32 a, uint32 b, uint32 c, uint32 d) {
			indices.push_back(a); indices.push_back(b); indices.push_back(d);
			indices.push_back(b); indices.push_back(c); indices.push_back(d);
		};

		for (uint32 i = i0; i < i0 + n; ++i)
		{
			const float ratio = (float)(i - i0) / (n - 1);
			float angle = startAngle + (endAngle - startAngle) * ratio;
			float cosAngle = cosf(angle);
			float sinAngle = sinf(angle);

			innerVertexIndices.push_back(i);
			innerVertexIndices.push_back(i + 2 * n);

			positions[i]         = vector3(innerRadius * cosAngle, innerRadius * sinAngle, +dz);
			positions[i + n]     = vector3(outerRadius * cosAngle, outerRadius * sinAngle, +dz);
			positions[i + 2 * n] = vector3(innerRadius * cosAngle, innerRadius * sinAngle, -dz);
			positions[i + 3 * n] = vector3(outerRadius * cosAngle, outerRadius * sinAngle, -dz);

			uvs[i]         = vector2(ratio, 0.0f);
			uvs[i + n]     = vector2(ratio, 0.25f);
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
		makeQuad(i0, i0 + 2 * n, i0 + 3 * n, i0 + n);
		makeQuad(n - 1 + i0, n - 1 + i0 + n, n - 1 + i0 + 3 * n, n - 1 + i0 + 2 * n);

		i0 += n * 4;
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

//////////////////////////////////////////////////////////////////////////

void SpaceshipActor::onSpawn()
{
	AssetReferenceWavefrontOBJ assetRef(OBJ_SPACESHIP_FILE, OBJ_SPACESHIP_DIR);
	gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRef, this, &SpaceshipActor::onLoadOBJ);
}

void SpaceshipActor::onLoadOBJ(OBJLoader* loader)
{
	LOG(LogInfo, "Load spaceship model");

	setStaticMesh(loader->craftMeshFromAllShapes());
}
