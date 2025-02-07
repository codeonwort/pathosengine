#include "world_physics.h"

#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/physics_component.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/static_mesh.h"
#include "pathos/material/material.h"

#include "badger/math/random.h"

// --------------------------------------------------------
// Constants

#define BALL_RANDOM_MATERIALS 1

static const vector3 CAMERA_POSITION = vector3(0.0f, 15.0f, 50.0f);
static const vector3 CAMERA_LOOK_AT  = vector3(0.0f, 0.0f, 0.0f);
static const vector3 SUN_DIRECTION   = vector3(1.0f, -1.0f, 0.0f);

static const float   SPHERE_RADIUS   = 2.0f;
static const float   GROUND_RADIUS   = 100.0f;

void World_Physics::onInitialize() {
	auto G_sphere = new SphereGeometry(SPHERE_RADIUS, 30);
#if !BALL_RANDOM_MATERIALS
	auto M_sphere = Material::createMaterialInstance("solid_color");
	M_sphere->setConstantParameter("albedo", vector3(0.9f));
	M_sphere->setConstantParameter("metallic", 0.0f);
	M_sphere->setConstantParameter("roughness", 1.0f);
	M_sphere->setConstantParameter("emissive", vector3(0.0f));
#endif

	auto G_ground = new SphereGeometry(GROUND_RADIUS, 60);
	auto M_ground = Material::createMaterialInstance("solid_color");
	M_ground->setConstantParameter("albedo", vector3(0.9f));
	M_ground->setConstantParameter("metallic", 0.0f);
	M_ground->setConstantParameter("roughness", 1.0f);
	M_ground->setConstantParameter("emissive", vector3(0.0f));

#if 1
	// Spheres
	for (int32 ix = -8; ix <= 8; ++ix) {
		for (int32 iz = -8; iz <= 8; ++iz) {
			float x = 2.2f * (float)ix * SPHERE_RADIUS;
			float y = 5.0f + Random() * 5.0f;
			float z = 2.2f * (float)iz * SPHERE_RADIUS;
			float elasticity = Random();
			float friction = Random();

#if BALL_RANDOM_MATERIALS
			auto M_sphere = Material::createMaterialInstance("solid_color");
			M_sphere->setConstantParameter("albedo", vector3(Random(), Random(), Random()));
			M_sphere->setConstantParameter("metallic", Random());
			M_sphere->setConstantParameter("roughness", Random());
			M_sphere->setConstantParameter("emissive", vector3(0.0f));
#endif

			auto sphere = spawnActor<StaticMeshActor>();
			sphere->setStaticMesh(new StaticMesh(G_sphere, M_sphere));
			sphere->setActorLocation(x, y, z);
			auto physComponent = new PhysicsComponent;
			physComponent->setMass(10.0f);
			physComponent->setElasticity(elasticity);
			physComponent->setFriction(friction);
			physComponent->setShapeSphere(SPHERE_RADIUS);
			sphere->registerComponent(physComponent);
		}
	}
#else
	// Sphere 1
	{
		auto sphere = spawnActor<StaticMeshActor>();
		sphere->setStaticMesh(new StaticMesh(G_sphere, M_sphere));
		sphere->setActorLocation(2.0f, 5.0f, -1.0f);
		auto physComponent = new PhysicsComponent;
		physComponent->setMass(10.0f);
		physComponent->setElasticity(0.5f);
		physComponent->setFriction(0.5f);
		physComponent->setShapeSphere(SPHERE_RADIUS);
		sphere->registerComponent(physComponent);
	}
	// Sphere 2
	{
		auto sphere = spawnActor<StaticMeshActor>();
		sphere->setStaticMesh(new StaticMesh(G_sphere, M_sphere));
		sphere->setActorLocation(-2.0f, 6.0f, -1.0f);
		auto physComponent = new PhysicsComponent;
		physComponent->setMass(10.0f);
		physComponent->setElasticity(0.2f);
		physComponent->setFriction(0.5f);
		physComponent->forceLinearVelocity(vector3(0.0f, -50.0f, 0.0f));
		physComponent->setShapeSphere(SPHERE_RADIUS);
		sphere->registerComponent(physComponent);
	}
#endif
	// Ground
	for (int32 ix = -1; ix <= 1; ++ix) {
		for (int32 iz = -1; iz <= 1; ++iz) {
			float x = (float)ix * GROUND_RADIUS;
			float z = (float)iz * GROUND_RADIUS;

			auto ground = spawnActor<StaticMeshActor>();
			ground->setStaticMesh(new StaticMesh(G_ground, M_ground));
			ground->setActorLocation(x, -GROUND_RADIUS - 10.0f, z);
			auto physComponent2 = new PhysicsComponent;
			physComponent2->setInfiniteMass();
			physComponent2->setElasticity(1.0f);
			physComponent2->setFriction(0.5f);
			physComponent2->setShapeSphere(GROUND_RADIUS);
			ground->registerComponent(physComponent2);
		}
	}

	auto sky = spawnActor<SkyAtmosphereActor>();

	auto sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(SUN_DIRECTION);

	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
}

void World_Physics::onTick(float deltaSeconds) {

}
