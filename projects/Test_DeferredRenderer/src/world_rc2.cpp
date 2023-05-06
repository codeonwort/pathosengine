#include "world_rc2.h"
#include "player_controller.h"

#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/mesh.h"
#include "pathos/material/material.h"

#define CAMERA_POSITION    vector3(0.0f, 0.0f, 30.0f)
#define CAMERA_LOOK_AT     vector3(0.0f, 0.0f, 0.0f)
#define CAMERA_UP          vector3(0.0f, 1.0f, 0.0f)

#define SUN_DIRECTION      glm::normalize(vector3(0.5f, -0.5f, -1.0f))
#define SUN_ILLUMINANCE    (10.0f * vector3(1.0f, 1.0f, 1.0f))

#define SQUARE_DIAMOND_COUNT   5
#define SQUARE_DIAMOND_CENTER  vector3(0.0f, 0.0f, 0.0f)
#define SQUARE_DIAMOND_Z_DELTA 20.0f
#define SQUARE_DIAMOND_R0      10.0f
#define SQUARE_DIAMOND_D0      4.0f
#define SQUARE_DIAMOND_SCALER  1.2f
#define SQUARE_DIAMOND_ROLL    15.0f

// -----------------------------------------------------------------------
// World_RC2

void World_RC2::onInitialize() {
	playerController = spawnActor<PlayerController>();

	sunActor = spawnActor<DirectionalLightActor>();
	sunActor->setDirection(SUN_DIRECTION);
	sunActor->setIlluminance(SUN_ILLUMINANCE);

	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, CAMERA_UP);

	squareDiamonds.reserve(SQUARE_DIAMOND_COUNT);
	vector3 diamondAlbedo(0.0f, 0.0f, 0.0f);
	for (size_t i = 0; i < SQUARE_DIAMOND_COUNT; ++i) {
		SquareDiamondActor* diamond = spawnActor<SquareDiamondActor>();

		diamondAlbedo.r += 0.27f;
		diamondAlbedo.g += 0.61f;
		diamondAlbedo.b += 0.48f;
		diamondAlbedo = glm::mod(diamondAlbedo, vector3(1.0f));
		diamond->buildMesh(diamondAlbedo, SQUARE_DIAMOND_R0, SQUARE_DIAMOND_D0);

		vector3 location = SQUARE_DIAMOND_CENTER;
		location.z -= SQUARE_DIAMOND_Z_DELTA * i;
		diamond->setActorLocation(location);

		diamond->setActorScale(1.0f + i * SQUARE_DIAMOND_SCALER);
		diamond->setActorRotation(Rotator(0.0f, 0.0f, i * SQUARE_DIAMOND_ROLL));

		squareDiamonds.push_back(diamond);
	}
}

void World_RC2::onTick(float deltaSeconds) {
	for (size_t i = 0; i < squareDiamonds.size(); ++i) {
		Actor* diamond = squareDiamonds[i];

		Rotator rotation = diamond->getActorRotation();
		rotation.roll += deltaSeconds * 5.0f * ((i & 1) ? 1 : -1);
		diamond->setActorRotation(rotation);
	}
}

// -----------------------------------------------------------------------
// SquareDiamondActor

struct Subdiv {
	vector3 center;
	vector3 halfSize;
};

SquareDiamondActor::SquareDiamondActor() {}

void SquareDiamondActor::buildMesh(const vector3& albedo, float R, float D) {
	std::vector<Subdiv> queue;
	queue.push_back(Subdiv{ vector3(-R + 0.5f * D, 0.5f * D, 0.0f),  vector3(0.5f * D, R - 0.5f * D, 0.5f * D) });
	queue.push_back(Subdiv{ vector3(0.5f * D, R - 0.5f * D, 0.0f),   vector3(R - 0.5f * D, 0.5f * D, 0.5f * D) });
	queue.push_back(Subdiv{ vector3(R - 0.5f * D, -0.5f * D, 0.0f),  vector3(0.5f * D, R - 0.5f * D, 0.5f * D) });
	queue.push_back(Subdiv{ vector3(-0.5f * D, -R + 0.5f * D, 0.0f), vector3(R - 0.5f * D, 0.5f * D, 0.5f * D) });
	
	// #wip: Subdivision here
	{
		// ...
	}

	auto geometry = new CubeGeometry(vector3(1.0f));
	geometries.push_back(geometry);

	auto material = Material::createMaterialInstance("solid_color");
	material->setConstantParameter("albedo", albedo);
	material->setConstantParameter("metallic", 0.0f);
	material->setConstantParameter("roughness", 1.0f);
	material->setConstantParameter("emissive", vector3(0.0f, 0.0f, 0.0f));
	materials.push_back(material);

	Mesh* staticMesh = new Mesh(geometry, material);
	staticMeshAssets.push_back(staticMesh);

	for (size_t i = 0; i < queue.size(); ++i) {
		StaticMeshComponent* smc = new StaticMeshComponent;

		smc->setStaticMesh(staticMesh);

		smc->setLocation(queue[i].center);
		smc->setScale(queue[i].halfSize);

		registerComponent(smc);
		smc->setTransformParent(getRootComponent());
		staticMeshComponents.push_back(smc);
	}
}
