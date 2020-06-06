#include "world_rc1.h"
#include "player_controller.h"

#include "pathos/render/sky_ansel.h"
#include "pathos/loader/imageloader.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/light/directional_light_actor.h"

const vector3       SUN_DIRECTION        = glm::normalize(vector3(0.0f, -1.0f, 0.0f));
const vector3       SUN_RADIANCE         = 1.2f * vector3(1.0f, 1.0f, 1.0f);

void World_RC1::onInitialize()
{
	playerController = spawnActor<PlayerController>();
	setupSky();
	setupScene();
}

void World_RC1::onTick(float deltaSeconds)
{
	//
}

void World_RC1::setupSky()
{
	GLuint equirectangularMap = pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr"));
	GLuint cubemapForIBL = IrradianceBaker::bakeCubemap(equirectangularMap, 512);
	glObjectLabel(GL_TEXTURE, equirectangularMap, -1, "Texture IBL: equirectangularMap");
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

	scene.sky = new AnselSkyRendering(pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr")));
}

void World_RC1::setupScene()
{
	DirectionalLightActor* dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setLightParameters(SUN_DIRECTION, SUN_RADIANCE);


	auto geom_sphere = new SphereGeometry(5.0f, 30);
	geom_sphere->calculateTangentBasis();

	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(2.0f, 0.2f, 0.2f);
		color->setMetallic(0.2f);
		color->setRoughness(0.1f);
	}

	//StaticMeshActor* godRaySource = spawnActor<StaticMeshActor>();
	//godRaySource->setStaticMesh(new Mesh(geom_sphere, material_color));
	//godRaySource->setActorScale(20.0f);
	//godRaySource->setActorLocation(vector3(0.0f, 300.0f, -500.0f));
	//godRaySource->getStaticMeshComponent()->castsShadow = false;
	//getScene().godRaySource = godRaySource->getStaticMeshComponent();
}
