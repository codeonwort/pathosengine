#include "world_game1.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/loader/scene_loader.h"
#include "pathos/render/atmosphere.h"
#include "pathos/render/skybox.h"
#include "pathos/render/sky_ansel.h"

const vector3       CAMERA_POSITION      = vector3(0.0f, 0.0f, 50.0f);
const vector3       CAMERA_LOOK_AT       = vector3(0.0f, 0.0f, 0.0f);

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
}

void World_Game1::onTick(float deltaSeconds)
{
	vector3 loc = pointLight0->getActorLocation();
	loc.x = 10.0f * ::sinf(gEngine->getWorldTime());
	pointLight0->setActorLocation(loc);
	PointLightComponent* p = static_cast<PointLightComponent*>(pointLight0->getRootComponent());
	p->color.g = (1.0f + ::cosf(gEngine->getWorldTime())) * 10.0f;
}

void World_Game1::prepareAssets()
{
	auto M_color = new ColorMaterial;
	M_color->setAlbedo(1.0f, 0.1f, 0.1f);
	M_color->setMetallic(0.0f);
	M_color->setRoughness(0.2f);

	auto G_sphere = new SphereGeometry(1.0f, 30);

	sphereMesh = new Mesh(G_sphere, M_color);
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

	SceneLoader sceneLoader;
	sceneLoader.loadSceneDescription(this, "resources/racing_game/test_scene.json", binder);

	setupScene();
}

void World_Game1::setupScene()
{
	sphere0->setStaticMesh(sphereMesh);
}
