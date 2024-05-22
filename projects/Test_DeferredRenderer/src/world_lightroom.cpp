#include "world_lightroom.h"
#include "player_controller.h"

#include "pathos/loader/gltf_loader.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/point_light_actor.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/mesh.h"
#include "pathos/util/log.h"
#include "pathos/engine.h"
#include "pathos/console.h"

#include "badger/math/random.h"


#define MODEL_FILEPATH                    "resources/lightroom/LightRoom.gltf"
#define POINT_LIGHT_COLOR                 (vector3(255, 169, 87) / 255.0f)
#define POINT_LIGHT_INTENSITY             5.0f * (800.0f / (3.14f * 4.0f))
#define POINT_LIGHT_ATTENUATION_RADIUS    20.0f
#define SUN_DIRECTION                     glm::normalize(vector3(0.0f, -1.0f, -1.0f))
#define SUN_COLOR                         vector3(1.0f, 1.0f, 1.0f)
#define SUN_ILLUMINANCE                   0.5f

// #wip-skyocclusion: Sky light leaks into interior.
#define CREATE_SKY_ACTOR                  1

// --------------------------------------------------------
// World

void World_LightRoom::onInitialize() {
	playerController = spawnActor<PlayerController>();

#if CREATE_SKY_ACTOR
	auto skyAtmosphere = spawnActor<SkyAtmosphereActor>();
#endif

	AssetReferenceGLTF assetRef(MODEL_FILEPATH);
	gEngine->getAssetStreamer()->enqueueGLTF(assetRef, this, &World_LightRoom::onLoadGLTF, 0);

	gConsole->addLine(L"r.omnishadow.size 2048");
}

void World_LightRoom::onTick(float deltaSeconds) {
	if (ballComponent != nullptr) {
		Rotator R = ballComponent->getRotation();
		R.yaw += 12.7f * deltaSeconds;
		R.pitch += 7.2f * deltaSeconds;
		R.roll += 3.5f * deltaSeconds;
		ballComponent->setRotation(R);
	}

	vector3 cameraPos = getCamera().getPosition();
	for (auto i = 0; i < fractures.size(); ++i) {
		vector3 pos = fractureOrigins[i];
		float dist = glm::distance(vector2(pos.x, pos.z), vector2(cameraPos.x, cameraPos.z));
		pos.y += 4.0f * glm::clamp((dist - 2.0f) / 8.0f, -0.2f, 1.0f);
		fractureTargets[i] += (pos - fractureTargets[i]) * 0.1f;
		//pos = fractureOrigins[i] + (fractureTargets[i] - fractureOrigins[i]) * 0.1f;
		fractures[i]->setLocation(fractureTargets[i]);
	}

	float worldTime = gEngine->getWorldTime();
	for (auto i = 0; i < leafComponents.size(); ++i) {
		float t = 0.5f + 0.5f * std::cosf(worldTime + (float(i) * 1.57f));
		leafComponents[i]->setLocation(glm::mix(leafOrigins[i], leafTargets[i], t));
	}
}

void World_LightRoom::onLoadGLTF(GLTFLoader* loader, uint64 payload) {
	if (loader->isValid() == false) {
		wchar_t msg[256];
		swprintf_s(msg, L"Failed to load: %S", MODEL_FILEPATH);
		gConsole->addLine(msg, false, true);
		return;
	}

	Actor* newActor = spawnActor<Actor>();
	std::vector<SceneComponent*> components;
	loader->attachToActor(newActor, &components);

	fractures.clear();
	fractureOrigins.clear();
	fractureTargets.clear();

	auto M_rectLight = Material::createMaterialInstance("solid_color");
	M_rectLight->setConstantParameter("albedo", vector3(0.0f));
	M_rectLight->setConstantParameter("metallic", 0.0f);
	M_rectLight->setConstantParameter("roughness", 1.0f);
	M_rectLight->setConstantParameter("emissive", vector3(100.0f, 60.0f, 30.0f));

	auto M_fracture = Material::createMaterialInstance("solid_color");
	M_fracture->setConstantParameter("albedo", vector3(0.2f));
	M_fracture->setConstantParameter("metallic", 0.0f);
	M_fracture->setConstantParameter("roughness", 1.0f);
	M_fracture->setConstantParameter("emissive", vector3(0.0f));

	struct LeafMarker {
		vector3 center;
		float radius;
	};
	std::vector<LeafMarker> leafMarkers;

	for (size_t i = 0; i < loader->numModels(); ++i) {
		const auto& modelDesc = loader->getModel(i);
		if (modelDesc.name == "PointLight") {
			PointLightComponent* light = static_cast<PointLightComponent*>(components[i]);
			light->sourceRadius = 0.5f;
		} else if (modelDesc.name == "Camera") {
			getCamera().moveToPosition(modelDesc.translation);
			getCamera().rotatePitch(modelDesc.rotation.pitch);
			getCamera().rotateYaw(modelDesc.rotation.yaw);
		} else if (modelDesc.name == "Ball") {
			ballComponent = static_cast<StaticMeshComponent*>(components[i]);
		} else if (modelDesc.name.find("RectLightMarker") != std::string::npos) {
			// #wip-light: Wanna place rect light but I can't align rotation.
			auto smc = static_cast<StaticMeshComponent*>(components[i]);
			smc->getStaticMesh()->setMaterial(0, M_rectLight);
			smc->castsShadow = false;
		} else if (modelDesc.name.find("Fracture") != std::string::npos) {
			auto smc = static_cast<StaticMeshComponent*>(components[i]);
			smc->getStaticMesh()->setMaterial(0, M_fracture);
			smc->castsShadow = false;
			fractures.push_back(smc);
			vector3 pos = components[i]->getLocation();
			pos.y += -0.1f + 0.2f * Random();
			fractureOrigins.push_back(pos);
			fractureTargets.push_back(pos);
		} else if (modelDesc.name.find("LeafMarker") != std::string::npos) {
			auto smc = static_cast<StaticMeshComponent*>(components[i]);
			smc->setVisibility(false);
			leafMarkers.push_back({ smc->getLocation(), smc->getScale().x });
		}
	}

	// Spawn leaf lights
	{
		const int NUM_LEAF_LIGHTS = 5000;

		auto M_leafLight = Material::createMaterialInstance("solid_color");
		M_leafLight->setConstantParameter("albedo", vector3(0.0f));
		M_leafLight->setConstantParameter("metallic", 0.0f);
		M_leafLight->setConstantParameter("roughness", 1.0f);
		M_leafLight->setConstantParameter("emissive", vector3(100.0f, 60.0f, 30.0f));

		auto G_leafLight = new SphereGeometry(1.0f, 20);

		float totalLeafWeight = 0.0f;
		size_t numMarkers = leafMarkers.size();
		std::vector<float> leafWeights(numMarkers);
		for (auto i = 0; i < numMarkers; ++i) {
			float w = leafMarkers[i].radius;
			leafWeights[i] = w;
			totalLeafWeight += w;
		}
		for (auto i = 0; i < numMarkers; ++i) {
			leafWeights[i] /= totalLeafWeight;
		}

		leafComponents.resize(NUM_LEAF_LIGHTS);
		leafOrigins.resize(NUM_LEAF_LIGHTS);
		leafTargets.resize(NUM_LEAF_LIGHTS);
		for (auto i = 0; i < NUM_LEAF_LIGHTS; ++i) {
			float r = Random();
			size_t j;
			for (j = 0; j < numMarkers; ++j) {
				if (r < leafWeights[j]) {
					break;
				}
				r -= leafWeights[j];
			}

			float lightRange = 0.05f + 0.05f * Random();
			vector3 lightCenter = leafMarkers[j].center + (0.1f * RandomInUnitSphere()) + (leafMarkers[j].radius * RandomInUnitSphere());

			auto sphere = spawnActor<StaticMeshActor>();
			sphere->setStaticMesh(new Mesh(G_leafLight, M_leafLight));
			sphere->setActorLocation(lightCenter);
			sphere->setActorScale(lightRange);
			sphere->getStaticMeshComponent()->castsShadow = false;

			leafOrigins[i] = lightCenter;
			leafTargets[i] = lightCenter + 0.1f * RandomInUnitSphere();
			leafComponents[i] = sphere->getStaticMeshComponent();
		}
	}
}
