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
	//: sun(nullptr)
	: sphere0(nullptr)
{
}

void World_Game1::onInitialize()
{
	SCOPED_CPU_COUNTER(World_Game1_initialize);

	SceneLoader sceneLoader;
	sceneLoader.loadSceneDescription(this, "resources/racing_game/test_scene.json");

	setupScene();
}

void World_Game1::onTick(float deltaSeconds)
{
}

void World_Game1::setupScene()
{
	auto M_color = new ColorMaterial;
	M_color->setAlbedo(1.0f, 0.0f, 0.0f);
	M_color->setMetallic(0.0f);
	M_color->setRoughness(0.2f);

	auto G_sphere = new SphereGeometry(1.0f, 30);

	//sun = spawnActor<DirectionalLightActor>();

	sphere0 = spawnActor<StaticMeshActor>();
	sphere0->setStaticMesh(new Mesh(G_sphere, M_color));
	sphere0->setActorScale(15.0f);
}
