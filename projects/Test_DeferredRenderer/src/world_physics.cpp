#include "world_physics.h"

#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/physics_component.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/mesh.h"
#include "pathos/material/material.h"

void World_Physics::onInitialize() {
	auto G_sphere = new SphereGeometry(1.0f, 30);
	auto M_sphere = Material::createMaterialInstance("solid_color");
	M_sphere->setConstantParameter("albedo", vector3(0.9f));
	M_sphere->setConstantParameter("metallic", 0.0f);
	M_sphere->setConstantParameter("roughness", 1.0f);
	M_sphere->setConstantParameter("emissive", vector3(0.0f));

	auto G_ground = new SphereGeometry(100.0f, 60);
	auto M_ground = Material::createMaterialInstance("solid_color");
	M_ground->setConstantParameter("albedo", vector3(0.9f));
	M_ground->setConstantParameter("metallic", 0.0f);
	M_ground->setConstantParameter("roughness", 1.0f);
	M_ground->setConstantParameter("emissive", vector3(0.0f));

	auto sphere = spawnActor<StaticMeshActor>();
	sphere->setStaticMesh(new Mesh(G_sphere, M_sphere));
	auto physComponent = new PhysicsComponent;
	physComponent->setMass(10.0f);
	sphere->registerComponent(physComponent);

	auto ground = spawnActor<StaticMeshActor>();
	ground->setStaticMesh(new Mesh(G_ground, M_ground));
	ground->setActorLocation(0.0f, -110.0f, 0.0f);
	auto physComponent2 = new PhysicsComponent;
	physComponent2->setInfiniteMass();
	ground->registerComponent(physComponent2);

	auto sky = spawnActor<SkyAtmosphereActor>();

	auto sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(vector3(1.0f, -1.0f, 0.0f));

	getCamera().lookAt(vector3(0.0f, 0.0f, 20.0f), vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f));
}

void World_Physics::onTick(float deltaSeconds) {

}
