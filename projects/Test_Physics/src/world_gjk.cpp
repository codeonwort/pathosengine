#include "world_gjk.h"
#include "player_controller.h"

#include "pathos/mesh/static_mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"

#include "badger/physics/collision.h"

#define CAMERA_ORIGIN   vector3(0.0f, 2.0f, 10.0f)
#define CAMERA_LOOKAT   vector3(0.0f, 0.0f, 0.0f)
#define SUN_DIRECTION   glm::normalize(vector3(0.0f, -1.0f, -1.0f))
#define SUN_COLOR       vector3(1.0f, 1.0f, 1.0f)
#define SUN_ILLUMINANCE 2.5f

void World_GJK::onInitialize() {
	getCamera().lookAt(CAMERA_ORIGIN, CAMERA_LOOKAT, vector3(0.0f, 1.0f, 0.0f));

	auto sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(SUN_DIRECTION);
	sun->setColorAndIlluminance(SUN_COLOR, SUN_ILLUMINANCE);

	auto sky = spawnActor<SkyAtmosphereActor>();

	MeshGeometry* boxGeometry = new CubeGeometry(vector3(1.0f));

	materialNoHit = Material::createMaterialInstance("solid_color");
	materialNoHit->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	materialNoHit->setConstantParameter("metallic", 0.0f);
	materialNoHit->setConstantParameter("roughness", 1.0f);
	materialNoHit->setConstantParameter("emissive", vector3(0.0f));

	materialOnHit = Material::createMaterialInstance("solid_color");
	materialOnHit->setConstantParameter("albedo", vector3(0.9f, 0.2f, 0.2f));
	materialOnHit->setConstantParameter("metallic", 0.0f);
	materialOnHit->setConstantParameter("roughness", 1.0f);
	materialOnHit->setConstantParameter("emissive", vector3(0.0f));

	StaticMesh* meshA = new StaticMesh(boxGeometry, materialNoHit);
	StaticMesh* meshB = new StaticMesh(boxGeometry, materialNoHit);

	modelA = spawnActor<StaticMeshActor>();
	modelB = spawnActor<StaticMeshActor>();

	modelA->setStaticMesh(meshA);
	modelA->setActorLocation(vector3(-2.0f, 0.0f, 0.0f));

	modelB->setStaticMesh(meshB);
	modelB->setActorLocation(vector3(2.0f, 0.0f, 0.0f));

	bodyA.setShape(new badger::physics::ShapeBox(vector3(2.0f)));
	bodyB.setShape(new badger::physics::ShapeBox(vector3(2.0f)));

	controller = spawnActor<PlayerController>();
	controller->setControlTarget(modelA);
}

void World_GJK::onTick(float deltaSeconds) {
	bodyA.setPosition(modelA->getActorLocation());
	bodyB.setPosition(modelB->getActorLocation());

	badger::physics::Contact contact;
	bool hit = badger::physics::intersect(&bodyA, &bodyB, contact);

	Material* material = hit ? materialOnHit : materialNoHit;
	modelA->getStaticMeshComponent()->getStaticMesh()->getLOD(0).setMaterial(0, material);
	modelB->getStaticMeshComponent()->getStaticMesh()->getLOD(0).setMaterial(0, material);
}
