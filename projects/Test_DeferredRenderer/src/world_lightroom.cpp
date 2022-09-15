#include "world_lightroom.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/light/point_light_actor.h"
#include "pathos/light/directional_light_actor.h"

#if SHARED_PTR_ACTORS
	#define TEMP_SPAWN_ACTOR(T) sharedPtr<T>(spawnActor<T>())
#else
	#define TEMP_SPAWN_ACTOR(T) spawnActor<T>()
#endif

// --------------------------------------------------------
// Constants

static const vector3 CAMERA_POSITION    = vector3(50.0f, 30.0f, 70.0f);
static const vector3 CAMERA_LOOK_AT     = vector3(0.0f, 10.0f, 0.0f);
static const vector3 SUN_DIRECTION      = glm::normalize(vector3(0.0f, -1.0f, -1.0f));
static const vector3 SUN_RADIANCE       = 2.0f * vector3(1.0f, 1.0f, 1.0f);

// --------------------------------------------------------
// World

void World_LightRoom::onInitialize() {
	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
	setupScene();
}

void World_LightRoom::onTick(float deltaSeconds) {
	//
}

void World_LightRoom::setupScene() {
	// --------------------------------------------------------
	// Ground

	MeshGeometry* G_ground = new PlaneGeometry(1000.0f, 1000.0f, 10, 10);

	ColorMaterial* M_ground = new ColorMaterial;
	M_ground->setAlbedo(0.33f, 0.22f, 0.18f); // brown
	M_ground->setRoughness(0.8f);
	M_ground->setMetallic(0.0f);

	ground = TEMP_SPAWN_ACTOR(StaticMeshActor);
	ground->setStaticMesh(new Mesh(G_ground, M_ground));
	ground->setActorLocation(0.0f, 0.0f, 0.0f);
	ground->setActorRotation(Rotator(0.0f, -90.0f, 0.0f));

	// --------------------------------------------------------
	// Box

	const float boxHalfSize = 10.0f;

	MeshGeometry* G_box = new CubeGeometry(vector3(boxHalfSize));

	ColorMaterial* M_box = new ColorMaterial;
	M_box->setAlbedo(0.9f, 0.9f, 0.9f);
	M_box->setRoughness(1.0f);
	M_box->setMetallic(0.0f);

	box = TEMP_SPAWN_ACTOR(StaticMeshActor);
	box->setStaticMesh(new Mesh(G_box, M_box));
	box->setActorLocation(0.0f, boxHalfSize, 0.0f);
	box->setActorScale(vector3(1.0f, 3.0f, 0.5f));

	// --------------------------------------------------------
	// Lights

	sun = TEMP_SPAWN_ACTOR(DirectionalLightActor);
	sun->setLightParameters(SUN_DIRECTION, SUN_RADIANCE);

	pointLight0 = TEMP_SPAWN_ACTOR(PointLightActor);
	pointLight0->setActorLocation(boxHalfSize * 2.5f, boxHalfSize, 0.0f);
	pointLight0->setLightParameters(vector3(10.0f, 0.0f, 0.0f), 1000.0f);

	MeshGeometry* G_pointLightGizmo = new SphereGeometry(1.0f, 6);
	ColorMaterial* M_pointLightGizmo = new ColorMaterial;
	M_pointLightGizmo->setAlbedo(0.0f, 0.0f, 0.0f);
	M_pointLightGizmo->setEmissive(0.9f, 0.9f, 0.0f);
	pointLight0Gizmo = TEMP_SPAWN_ACTOR(StaticMeshActor);
	pointLight0Gizmo->setStaticMesh(new Mesh(G_pointLightGizmo, M_pointLightGizmo));
	pointLight0Gizmo->setActorLocation(pointLight0->getActorLocation());
}
