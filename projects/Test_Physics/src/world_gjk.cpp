#include "world_gjk.h"
#include "player_controller.h"

#include "pathos/mesh/static_mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/geometry_procedural.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"

#include "badger/physics/collision.h"

#define CAMERA_ORIGIN   vector3(0.0f, 2.0f, 7.0f)
#define CAMERA_LOOKAT   vector3(0.0f, 0.0f, 0.0f)
#define SUN_DIRECTION   glm::normalize(vector3(0.0f, -1.0f, -1.0f))
#define SUN_COLOR       vector3(1.0f, 1.0f, 1.0f)
#define SUN_ILLUMINANCE 2.5f

#define COMPLEX_SHAPE   0

void World_GJK::onInitialize() {
	getCamera().lookAt(CAMERA_ORIGIN, CAMERA_LOOKAT, vector3(0.0f, 1.0f, 0.0f));

	auto sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(SUN_DIRECTION);
	sun->setColorAndIlluminance(SUN_COLOR, SUN_ILLUMINANCE);

	auto sky = spawnActor<SkyAtmosphereActor>();

#if COMPLEX_SHAPE
	ProceduralGeometry* geometry = new ProceduralGeometry;
	std::vector<vector3> vertices = {
		vector3(-1.0f, 0.0f, 0.0f),
		vector3(1.0f, 0.0f, 0.0f),
		vector3(0.0f, 1.0f, 0.0f),
		vector3(0.0f, 0.0f, 1.0f),
	};
	{
		uint32 indices[] = {
			0, 1, 2,
			0, 1, 3,
			1, 2, 3,
			2, 0, 3,
		};
		for (uint32 i = 0; i < _countof(indices); i += 3) {
			geometry->addTriangle(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
		}
		geometry->upload();
		geometry->calculateNormals();
		geometry->calculateTangentBasis();
	}
#else
	//MeshGeometry* geometry = new CubeGeometry(vector3(1.0f));
	MeshGeometry* geometry = new SphereGeometry(SphereGeometry::Input{ 1.0f, 6 });
	SphereGeometry::Output geomOutput;
	SphereGeometry::generate({ 1.0f, 6 }, geomOutput);
	std::vector<vector3> geomVertices(geomOutput.positions.size() / 3);
	for (size_t i = 0; i < geomVertices.size(); ++i) {
		geomVertices[i].x = geomOutput.positions[i * 3 + 0];
		geomVertices[i].y = geomOutput.positions[i * 3 + 1];
		geomVertices[i].z = geomOutput.positions[i * 3 + 2];
	}
#endif

	materialNoHit = Material::createMaterialInstance("solid_color");
	materialNoHit->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	materialNoHit->setConstantParameter("metallic", 0.0f);
	materialNoHit->setConstantParameter("roughness", 1.0f);
	materialNoHit->setConstantParameter("emissive", vector3(0.0f));

	materialOnHit = Material::createMaterialInstance("translucent_color");
	materialOnHit->setConstantParameter("albedo", vector3(0.3f, 0.3f, 0.3f));
	materialOnHit->setConstantParameter("roughness", 1.0f);
	materialOnHit->setConstantParameter("transmittance", vector3(0.8f));
	//materialOnHit = Material::createMaterialInstance("solid_color");
	//materialOnHit->setConstantParameter("albedo", vector3(0.9f, 0.2f, 0.2f));
	//materialOnHit->setConstantParameter("metallic", 0.0f);
	//materialOnHit->setConstantParameter("roughness", 1.0f);
	//materialOnHit->setConstantParameter("emissive", vector3(0.0f));

	assetPtr<StaticMesh> meshA = makeAssetPtr<StaticMesh>(geometry, materialNoHit);
	assetPtr<StaticMesh> meshB = makeAssetPtr<StaticMesh>(geometry, materialNoHit);

	modelA = spawnActor<StaticMeshActor>();
	modelB = spawnActor<StaticMeshActor>();

	modelA->setStaticMesh(meshA);
	modelA->setActorLocation(vector3(-2.0f, 0.5f, 0.2f));

	modelB->setStaticMesh(meshB);
	modelB->setActorLocation(vector3(2.0f, 0.0f, 0.0f));

#if COMPLEX_SHAPE
	bodyA.setShape(new badger::physics::ShapeConvex(vertices));
	bodyB.setShape(new badger::physics::ShapeConvex(vertices));
#else
	bodyA.setShape(new badger::physics::ShapeConvex(geomVertices));
	bodyB.setShape(new badger::physics::ShapeConvex(geomVertices));
#endif

	{
		auto arrowG = new CubeGeometry(vector3(1.0f, 0.05f, 0.05f));
		auto arrowM = Material::createMaterialInstance("solid_color");
		arrowM->setConstantParameter("albedo", vector3(0.9f, 0.0f, 0.0f));
		arrowM->setConstantParameter("metallic", 0.0f);
		arrowM->setConstantParameter("roughness", 1.0f);
		arrowM->setConstantParameter("emissive", vector3(0.0f));
		assetPtr<StaticMesh> arrowMesh = makeAssetPtr<StaticMesh>(arrowG, arrowM);

		arrow = spawnActor<StaticMeshActor>();
		arrow->setStaticMesh(arrowMesh);
	}

	controller = spawnActor<PlayerController>();
	controller->setControlTarget(modelA);
}

void World_GJK::onTick(float deltaSeconds) {
	bodyA.setPosition(modelA->getActorLocation());
	bodyB.setPosition(modelB->getActorLocation());

	badger::physics::Contact contact;
	bool hit = badger::physics::intersect(&bodyA, &bodyB, contact);

	auto material = hit ? materialOnHit : materialNoHit;
	modelA->getStaticMeshComponent()->getStaticMesh()->getLOD(0).setMaterial(0, material);
	modelB->getStaticMeshComponent()->getStaticMesh()->getLOD(0).setMaterial(0, material);

	if (hit) {
		Rotator rot = Rotator::directionToYawPitch(contact.normal);
		arrow->setActorLocation(contact.surfaceA_WS);
		arrow->setActorRotation(rot);
		arrow->getStaticMeshComponent()->setVisibility(true);
	} else {
		arrow->getStaticMeshComponent()->setVisibility(false);
	}
}
