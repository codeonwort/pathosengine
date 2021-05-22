#include "world_game1.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/loader/scene_loader.h"

const vector3       CAMERA_POSITION      = vector3(0.0f, 0.0f, 50.0f);
const vector3       CAMERA_LOOK_AT       = vector3(0.0f, 0.0f, 0.0f);

World_Game1::World_Game1()
	: sun(nullptr)
	, pointLight0(nullptr)
	, sphere0(nullptr)
{
}

void World_Game1::onInitialize()
{
	SCOPED_CPU_COUNTER(World_Game1_initialize);

	ActorBinder binder;
	binder.addBinding("Sun", &sun);
	binder.addBinding("PointLight0", &pointLight0);
	SceneLoader sceneLoader;
	sceneLoader.loadSceneDescription(this, "resources/racing_game/test_scene.json", binder);

	setupScene();
}

void World_Game1::onTick(float deltaSeconds)
{
	vector3 loc = pointLight0->getActorLocation();
	loc.x = 10.0f * ::sinf(gEngine->getWorldTime());
	pointLight0->setActorLocation(loc);
	PointLightComponent* p = static_cast<PointLightComponent*>(pointLight0->getRootComponent());
	p->color.g = (1.0f + ::cosf(gEngine->getWorldTime())) * 10.0f;
}

void World_Game1::setupScene()
{
	auto M_color = new ColorMaterial;
	M_color->setAlbedo(1.0f, 0.0f, 0.0f);
	M_color->setMetallic(0.0f);
	M_color->setRoughness(0.2f);

	auto G_sphere = new SphereGeometry(1.0f, 30);

	sphere0 = spawnActor<StaticMeshActor>();
	sphere0->setStaticMesh(new Mesh(G_sphere, M_color));
	sphere0->setActorScale(15.0f);
}
