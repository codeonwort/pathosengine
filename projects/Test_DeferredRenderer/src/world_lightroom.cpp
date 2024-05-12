#include "world_lightroom.h"
#include "player_controller.h"

#include "pathos/loader/gltf_loader.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/point_light_actor.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/mesh/mesh.h"
#include "pathos/engine.h"
#include "pathos/console.h"

#define MODEL_FILEPATH                    "resources/lightroom/LightRoom.gltf"
#define POINT_LIGHT_COLOR                 (vector3(255, 169, 87) / 255.0f)
#define POINT_LIGHT_INTENSITY             5.0f * (800.0f / (3.14f * 4.0f))
#define POINT_LIGHT_ATTENUATION_RADIUS    20.0f
#define SUN_DIRECTION                     glm::normalize(vector3(0.0f, -1.0f, -1.0f))
#define SUN_COLOR                         vector3(1.0f, 1.0f, 1.0f)
#define SUN_ILLUMINANCE                   0.5f

// --------------------------------------------------------
// World

void World_LightRoom::onInitialize() {
	playerController = spawnActor<PlayerController>();

	// #wip-skyocclusion: Sky light leaks into interior.
	auto skyAtmosphere = spawnActor<SkyAtmosphereActor>();

	AssetReferenceGLTF assetRef(MODEL_FILEPATH);
	gEngine->getAssetStreamer()->enqueueGLTF(assetRef, this, &World_LightRoom::onLoadGLTF, 0);
}

void World_LightRoom::onTick(float deltaSeconds) {
	if (ballComponent != nullptr) {
		Rotator R = ballComponent->getRotation();
		R.yaw += 12.7f * deltaSeconds;
		R.pitch += 7.2f * deltaSeconds;
		R.roll += 3.5f * deltaSeconds;
		ballComponent->setRotation(R);
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

	for (size_t i = 0; i < loader->numModels(); ++i) {
		const auto& modelDesc = loader->getModel(i);
		if (modelDesc.name == "PointLight") {
			PointLightComponent* light = static_cast<PointLightComponent*>(components[i]);
			// Surprisingly every faces of Ball are parallel to view vector when rendering omni shadow maps
			// if Light and Ball have the same center.
			light->setLocation(light->getLocation() + vector3(0.05f, 0.05f, 0.05f));
			// #wip-light: source radius not parsed yet
			light->sourceRadius = 0.5f;
		} else if (modelDesc.name == "Camera") {
			getCamera().moveToPosition(modelDesc.translation);
			getCamera().rotatePitch(modelDesc.rotation.pitch);
			getCamera().rotateYaw(modelDesc.rotation.yaw);
		} else if (modelDesc.name == "Ball") {
			ballComponent = static_cast<StaticMeshComponent*>(components[i]);
			ballComponent->getStaticMesh()->doubleSided = true;
		}
	}
}
