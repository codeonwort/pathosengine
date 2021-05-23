#include "scene_loader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"
#include "pathos/actor/world.h"
#include "pathos/light/point_light_actor.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/render/atmosphere.h"

#include "badger/system/stopwatch.h"
#include <fstream>
#include <sstream>

namespace pathos {

	bool SceneLoader::loadSceneDescription(
		World* world,
		const char* inFilename,
		ActorBinder& actorBinder)
	{
		LOG(LogDebug, "Loading scene description...: %s", inFilename);

		Stopwatch timer;
		timer.start();

		std::string jsonString;
		if (!loadJSON(inFilename, jsonString)) {
			return false;
		}

		SceneDescription desc;
		if (!parseJSON(jsonString, desc)) {
			LOG(LogError, "Failed to parse: %s", inFilename);
			return false;
		}

		ActorMap actorMap;
		applyDescription(world, desc, actorMap);
		bindActors(desc, actorMap, actorBinder);

		LOG(LogDebug, "Loading done in %f ms", 1000.0f * timer.stop());

		return true;
	}

	bool SceneLoader::loadJSON(const char* inFilename, std::string& outJSON) {
		std::string filename = ResourceFinder::get().find(inFilename);
		if (filename.empty()) {
			LOG(LogError, "Invalid filepath: %s", inFilename);
			return false;
		}

		std::fstream fs;
		fs.open(filename);
		if (!fs.is_open()) {
			LOG(LogError, "Failed to open: %s", filename.c_str());
			return false;
		}

		std::stringstream ss;
		ss << fs.rdbuf();
		outJSON = ss.str();

		return true;
	}

	bool SceneLoader::parseJSON(const std::string& inJSON, SceneDescription& outDesc) {
		SceneDescriptionParser parser;
		return parser.parse(inJSON, outDesc);
	}

	void SceneLoader::applyDescription(World* world, const SceneDescription& sceneDesc, ActorMap& outActorMap) {
		// sky
		if (sceneDesc.skyAtmosphere.valid) {
			AtmosphereScattering* actor = world->spawnActor<AtmosphereScattering>();

			world->getScene().sky = actor;
			outActorMap.insert(std::make_pair(sceneDesc.skyAtmosphere.name, actor));
		}
		// directional lights
		for (const SceneDescription::DirLight& dirLight : sceneDesc.dirLights) {
			DirectionalLightActor* actor = world->spawnActor<DirectionalLightActor>();
			actor->setLightParameters(dirLight.direction, dirLight.radiance);

			outActorMap.insert(std::make_pair(dirLight.name, actor));
		}
		// point lights
		for (const SceneDescription::PointLight& pLight : sceneDesc.pointLights) {
			PointLightActor* actor = world->spawnActor<PointLightActor>();
			actor->setLightParameters(pLight.radiance, pLight.attenuationRadius,
				pLight.falloffExponent, pLight.castsShadow);
			actor->setActorLocation(pLight.location);

			outActorMap.insert(std::make_pair(pLight.name, actor));
		}
		// static meshes
		for (const SceneDescription::StaticMesh& sm : sceneDesc.staticMeshes) {
			StaticMeshActor* actor = world->spawnActor<StaticMeshActor>();
			actor->setActorLocation(sm.location);
			actor->setActorRotation(sm.rotation);
			actor->setActorScale(sm.scale);

			outActorMap.insert(std::make_pair(sm.name, actor));
		}
	}

	void SceneLoader::bindActors(SceneDescription& desc, const ActorMap& actorMap, ActorBinder& binder) {
		if (binder.bindings.size() == 0) {
			return;
		}

		// Find bindings between names in scene description and actors in world
		for (auto& it : binder.bindings) {
			const std::string& targetName = it.first;
			ActorBinder::Info& bindInfo = it.second;

			auto it = actorMap.find(targetName);
			if (it != actorMap.end()) {
				Actor* targetActor = it->second;
				*bindInfo.actor = targetActor;
				bindInfo.bound = true;
			}
		}

		// Validate if all actors are bound
		int32 countNotBound = 0;
		for (const auto& it : binder.bindings) {
			if (it.second.bound == false) {
				LOG(LogError, "Not bound: %s", it.first.c_str());
				++countNotBound;
			}
		}
		CHECKF(countNotBound == 0, "Not all actors are bound");
	}

}
