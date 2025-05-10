#include "world_rc2.h"
#include "player_controller.h"

#include "pathos/scene/skybox_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/static_mesh.h"
#include "pathos/material/material.h"

#include "badger/math/random.h"

#define CAMERA_POSITION          vector3(0.0f, 0.0f, 5.0f)
#define CAMERA_LOOK_AT           vector3(0.0f, 0.0f, 0.0f)
#define CAMERA_UP                vector3(0.0f, 1.0f, 0.0f)

#define SUN_DIRECTION            glm::normalize(vector3(0.5f, -0.5f, -1.0f))
#define SUN_COLOR                vector3(1.0f, 1.0f, 1.0f)
#define SUN_ILLUMINANCE          50.0f
#define SKY_INTENSITY            5.0f

#define SQUARE_DIAMOND_COUNT     6
#define SQUARE_DIAMOND_CENTER    vector3(0.0f, 0.0f, 0.0f)
#define SQUARE_DIAMOND_Z_SIZE    0.2f
#define SQUARE_DIAMOND_R0        4.0f
#define SQUARE_DIAMOND_R_INC     1.0f
#define SQUARE_DIAMOND_D_MIN     0.4f
#define SQUARE_DIAMOND_D_MAX     0.8f
#define SQUARE_DIAMOND_ROLL      1.0f
#define SQUARE_DIAMOND_SUBDIV    7
#define SQUARE_DIAMOND_MARGIN    0.01f
#define SQUARE_DIAMOND_MIN_SIZE  0.05f
#define SQUARE_DIAMOND_P_EMPTY   0.01f
#define SQUARE_DIAMOND_P_NODIV   0.15f

// -----------------------------------------------------------------------
// World_RC2

void World_RC2::onInitialize() {
	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, CAMERA_UP);

	playerController = spawnActor<PlayerController>();

	sunActor = spawnActor<DirectionalLightActor>();
	sunActor->setDirection(SUN_DIRECTION);
	sunActor->setColorAndIlluminance(SUN_COLOR, SUN_ILLUMINANCE);

	skyMaterial = Material::createMaterialInstance("skybox_rc2");
	skyMaterial->setConstantParameter("skyIntensity", SKY_INTENSITY);
	skyActor = spawnActor<SkyboxActor>();
	skyActor->setSkyboxMaterial(skyMaterial);

	squareDiamonds.reserve(SQUARE_DIAMOND_COUNT);
	for (size_t i = 0; i < SQUARE_DIAMOND_COUNT; ++i) {
		SquareDiamondActor* diamond = spawnActor<SquareDiamondActor>();

		float R = SQUARE_DIAMOND_R0;
		R += i * SQUARE_DIAMOND_R_INC;
		float D = R * glm::mix(SQUARE_DIAMOND_D_MIN, SQUARE_DIAMOND_D_MAX, (float)i / (SQUARE_DIAMOND_COUNT - 1));
		diamond->buildMesh(R, D);

		vector3 location = SQUARE_DIAMOND_CENTER;
		location.z -= i * 2.5f * SQUARE_DIAMOND_Z_SIZE;
		diamond->setActorLocation(location);

		diamond->setActorRotation(Rotator(0.0f, 0.0f, i * SQUARE_DIAMOND_ROLL));

		squareDiamonds.push_back(diamond);
	}
}

void World_RC2::onTick(float deltaSeconds) {
	for (size_t i = 0; i < squareDiamonds.size(); ++i) {
		Actor* diamond = squareDiamonds[i];

		Rotator rotation = diamond->getActorRotation();
		//rotation.roll += deltaSeconds * 5.0f * ((i & 1) ? 1 : -1);
		rotation.roll += deltaSeconds * 5.0f * glm::pow(1.1f, (float)i);
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

void SquareDiamondActor::buildMesh(float R, float D) {
	std::vector<Subdiv> queue1, queue2;
	queue1.push_back(Subdiv{ vector3(-R + 0.5f * D, 0.5f * D, 0.0f),  vector3(0.5f * D, R - 0.5f * D, SQUARE_DIAMOND_Z_SIZE) });
	queue1.push_back(Subdiv{ vector3(0.5f * D, R - 0.5f * D, 0.0f),   vector3(R - 0.5f * D, 0.5f * D, SQUARE_DIAMOND_Z_SIZE) });
	queue1.push_back(Subdiv{ vector3(R - 0.5f * D, -0.5f * D, 0.0f),  vector3(0.5f * D, R - 0.5f * D, SQUARE_DIAMOND_Z_SIZE) });
	queue1.push_back(Subdiv{ vector3(-0.5f * D, -R + 0.5f * D, 0.0f), vector3(R - 0.5f * D, 0.5f * D, SQUARE_DIAMOND_Z_SIZE) });
	
	// Subdivision
	bool queue1_is_input = true;
	for (int32 i = 0; i < SQUARE_DIAMOND_SUBDIV; ++i) {
		const std::vector<Subdiv>& inputQueue = queue1_is_input ? queue1 : queue2;
		std::vector<Subdiv>& outputQueue = queue1_is_input ? queue2 : queue1;

		outputQueue.clear();
		outputQueue.reserve(inputQueue.size() * 4);
		for (const Subdiv& inputRegion : inputQueue) {
			float e0 = Random();

			vector3 miniSize = vector3(0.5f, 0.5f, 1.0f) * inputRegion.halfSize;
			miniSize.x = miniSize.x - SQUARE_DIAMOND_MARGIN;
			miniSize.y = miniSize.y - SQUARE_DIAMOND_MARGIN;

			// Jitter Z
			vector3 inputCenter = inputRegion.center;
			inputCenter.z += 0.5f * (Random() - 0.5f) * miniSize.z;

			bool bDivX = true, bDivY = true;
			if (miniSize.x < SQUARE_DIAMOND_MIN_SIZE) {
				miniSize.x = SQUARE_DIAMOND_MIN_SIZE;
				bDivX = false;
			}
			if (miniSize.y < SQUARE_DIAMOND_MIN_SIZE) {
				miniSize.y = SQUARE_DIAMOND_MIN_SIZE;
				bDivY = false;
			}

			// Subdiv Z?
			if (!bDivX && !bDivY) {
				if (e0 < SQUARE_DIAMOND_P_EMPTY) {
					// empty
				} else {
					// don't subdivide
					outputQueue.push_back(inputRegion);
				}
			} else if (bDivX && bDivY) {
				if (e0 < SQUARE_DIAMOND_P_EMPTY) {
					// empty
				} else if (e0 < SQUARE_DIAMOND_P_NODIV) {
					outputQueue.push_back(inputRegion);
				} else {
					if (miniSize.x > miniSize.y || (miniSize.x == miniSize.y && e0 < 0.5f)) {
						outputQueue.push_back(Subdiv{ inputCenter + vector3(-1, 0, 0) * 0.5f * inputRegion.halfSize, vector3(1, 2, 1) * miniSize });
						outputQueue.push_back(Subdiv{ inputCenter + vector3(+1, 0, 0) * 0.5f * inputRegion.halfSize, vector3(1, 2, 1) * miniSize });
					} else {
						outputQueue.push_back(Subdiv{ inputCenter + vector3(0, -1, 0) * 0.5f * inputRegion.halfSize, vector3(2, 1, 1) * miniSize });
						outputQueue.push_back(Subdiv{ inputCenter + vector3(0, +1, 0) * 0.5f * inputRegion.halfSize, vector3(2, 1, 1) * miniSize });
					}
				}
			} else if (bDivX && !bDivY) {
				if (e0 < SQUARE_DIAMOND_P_EMPTY) {
					// empty
				} else if (e0 < SQUARE_DIAMOND_P_NODIV) {
					outputQueue.push_back(inputRegion);
				} else {
					outputQueue.push_back(Subdiv{ inputCenter + vector3(-1, 0, 0) * 0.5f * inputRegion.halfSize, vector3(1, 2, 1) * miniSize });
					outputQueue.push_back(Subdiv{ inputCenter + vector3(+1, 0, 0) * 0.5f * inputRegion.halfSize, vector3(1, 2, 1) * miniSize });
				}
			} else {
				if (e0 < SQUARE_DIAMOND_P_EMPTY) {
					// empty
				} else if (e0 < SQUARE_DIAMOND_P_NODIV) {
					outputQueue.push_back(inputRegion);
				} else {
					outputQueue.push_back(Subdiv{ inputCenter + vector3(0, -1, 0) * 0.5f * inputRegion.halfSize, vector3(2, 1, 1) * miniSize });
					outputQueue.push_back(Subdiv{ inputCenter + vector3(0, +1, 0) * 0.5f * inputRegion.halfSize, vector3(2, 1, 1) * miniSize });
				}
			}
		}

		queue1_is_input = !queue1_is_input;
	}
	std::vector<Subdiv>& finalQueue = queue1_is_input ? queue1 : queue2;

	auto geometry = makeAssetPtr<CubeGeometry>(vector3(1.0f));
	geometries.push_back(geometry);

	for (size_t i = 0; i < finalQueue.size(); ++i) {
		StaticMeshComponent* smc = new StaticMeshComponent;

		float e0 = Random();
		float e1 = Random();
		float e2 = Random();
		float e3 = Random();
		vector3 albedo = glm::mix(vector3(0.2f, 0.7f, 1.0f), vector3(1.0f, 0.5f, 0.0f), e0);
		vector3 emissive = (e1 <= 0.8f) ? vector3(0.0f) : (50.0f * e1 * albedo);
		float metallic = e2 < 0.5f ? 0.0f : 1.0f;
		float roughness = e3;

		auto material = Material::createMaterialInstance("solid_color");
		material->setConstantParameter("albedo", albedo);
		material->setConstantParameter("metallic", metallic);
		material->setConstantParameter("roughness", roughness);
		material->setConstantParameter("emissive", emissive);
		materials.push_back(material);

		assetPtr<StaticMesh> staticMesh = makeAssetPtr<StaticMesh>(geometry, material);
		staticMeshAssets.push_back(staticMesh);

		smc->setStaticMesh(staticMesh);

		smc->setLocation(finalQueue[i].center);
		smc->setScale(finalQueue[i].halfSize);

		registerComponent(smc);
		smc->setTransformParent(getRootComponent());
		staticMeshComponents.push_back(smc);
	}
}
