#include "world_physics.h"

#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/physics_component.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/mesh.h"
#include "pathos/material/material.h"

void World_Physics::onInitialize() {
	auto G_sphere = new SphereGeometry(0.5f, 30);
	auto M_sphere = Material::createMaterialInstance("solid_color");
	M_sphere->setConstantParameter("albedo", vector3(0.9f));
	M_sphere->setConstantParameter("metallic", 0.0f);
	M_sphere->setConstantParameter("roughness", 1.0f);
	M_sphere->setConstantParameter("emissive", vector3(0.0f));

	auto sphere = spawnActor<StaticMeshActor>();
	sphere->setStaticMesh(new Mesh(G_sphere, M_sphere));
	sphere->registerComponent(new PhysicsComponent);

	auto sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(vector3(1.0f, -1.0f, 0.0f));

	getCamera().lookAt(vector3(0.0f, 0.0f, 5.0f), vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f));
}

void World_Physics::onTick(float deltaSeconds) {

}
