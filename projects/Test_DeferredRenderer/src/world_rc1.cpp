#include "world_rc1.h"
#include "galaxy_gen.h"
#include "player_controller.h"

#include "pathos/render/sky_ansel.h"
#include "pathos/loader/imageloader.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/geometry_procedural.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/input/input_manager.h"

const vector3       SUN_DIRECTION        = glm::normalize(vector3(0.0f, -1.0f, 0.0f));
const vector3       SUN_RADIANCE         = 1.2f * vector3(1.0f, 1.0f, 1.0f);

void World_RC1::onInitialize()
{
	playerController = spawnActor<PlayerController>();
	setupSky();
	setupScene();

	ButtonBinding updateSky;
	updateSky.addInput(InputConstants::KEYBOARD_R);

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindButtonPressed("updateSky", updateSky, [this]()
		{
			updateStarfield();
		}
	);
}

void World_RC1::onTick(float deltaSeconds)
{
	for (uint32 i = 0; i < (uint32)rings.size(); ++i) {
		RingActor* ring = rings[i];
		Rotator rot = ring->getActorRotation();
		if (i % 2 == 0) {
			rot.pitch += 10.0f * deltaSeconds;
		} else {
			rot.yaw += 12.0f * deltaSeconds;
		}
		ring->setActorRotation(rot);
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
}

void World_RC1::setupScene()
{
	//////////////////////////////////////////////////////////////////////////
	// Light
	DirectionalLightActor* dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setLightParameters(SUN_DIRECTION, SUN_RADIANCE);

	//////////////////////////////////////////////////////////////////////////
	// Objects
	auto geom_sphere = new SphereGeometry(5.0f, 30);
	geom_sphere->calculateTangentBasis();

	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(2.0f, 0.2f, 0.2f);
		color->setMetallic(0.2f);
		color->setRoughness(0.1f);
	}

	sphere = spawnActor<StaticMeshActor>();
	sphere->setStaticMesh(new Mesh(geom_sphere, material_color));
	sphere->setActorScale(10.0f);

	constexpr uint32 numRings = 6;
	float ring_gap = 20.0f;
	float ring_width = 100.0f;
	float innerRadius = 150.0f;
	float outerRadius = innerRadius + ring_width;
	for (uint32 i = 0; i < numRings; ++i) {
		auto ring = spawnActor<RingActor>();
		ring->buildRing(innerRadius, outerRadius);
		innerRadius = outerRadius + ring_gap;
		outerRadius = innerRadius + ring_width;
		rings.push_back(ring);
	}

	StaticMeshActor* godRaySource = spawnActor<StaticMeshActor>();
	godRaySource->setStaticMesh(new Mesh(geom_sphere, material_color));
	godRaySource->setActorScale(20.0f);
	godRaySource->setActorLocation(vector3(0.0f, 300.0f, -5500.0f));
	godRaySource->getStaticMeshComponent()->castsShadow = false;
	getScene().godRaySource = godRaySource->getStaticMeshComponent();
}

void World_RC1::updateStarfield()
{
	gEngine->execute("recompile_shaders");
	GalaxyGenerator::createStarField(starfield, 2048, 1024);
}

//////////////////////////////////////////////////////////////////////////

RingActor::RingActor()
{
	G = new ProceduralGeometry;
	M = new ColorMaterial;
	M->setAlbedo(1.8f, 1.8f, 3.0f);
	M->setRoughness(1.0f);
	M->setMetallic(0.0f);
	
	setStaticMesh(new Mesh(G, M));
}

void RingActor::buildRing(float innerRadius, float outerRadius)
{
	G->clear();

	constexpr uint32 numSegments = 60;
	{
		constexpr auto n = numSegments;
		std::vector<vector3> vertices(n * 2);
		std::vector<vector2> uvs(n * 2);
		std::vector<uint32> indices;
		indices.reserve(n * 6);
		
		for (uint32 i = 0; i < numSegments; ++i)
		{
			float angle = 3.141592f * 2.0f * (float)i / n;
			vector3 dir(cosf(angle), sinf(angle), 0.0f);
			vertices[i] = innerRadius * dir;
			vertices[i + n] = outerRadius * dir;
			uvs[i] = vector2((float)i / n, 0.0f);
			uvs[i + n] = vector2((float)i / n, 1.0f);
			if (i != numSegments - 1) {
				indices.push_back(i);
				indices.push_back(i + n);
				indices.push_back(i + 1);
				indices.push_back(i + n);
				indices.push_back(i + n + 1);
				indices.push_back(i + 1);
			} else {
				indices.push_back(i);
				indices.push_back(i + n);
				indices.push_back(0);
				indices.push_back(i + n);
				indices.push_back(n);
				indices.push_back(0);
			}
		}

		G->updatePositionData((float*)vertices.data(), (uint32)(vertices.size() * sizeof(float) * 3));
		G->updateUVData((float*)uvs.data(), (uint32)(uvs.size() * sizeof(float) * 2));
		G->updateIndexData(indices.data(), (uint32)indices.size());
		G->calculateNormals();
		G->calculateTangentBasis();
	}

	getStaticMeshComponent()->getStaticMesh()->doubleSided = true;
}
