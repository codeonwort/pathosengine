#include "scene_loader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"
#include "pathos/actor/world.h"
#include "pathos/light/directional_light_actor.h"

#include <fstream>
#include <sstream>

namespace pathos {

	bool SceneLoader::loadSceneDescription(
		World* world,
		const char* inFilename,
		ActorBinder& actorBinder)
	{
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
		// directional lights
		for (const SceneDescription::DirLight& dirLight : sceneDesc.dirLights) {
			DirectionalLightActor* actor = world->spawnActor<DirectionalLightActor>();
			actor->setLightParameters(dirLight.direction, dirLight.radiance);

			outActorMap.insert(std::make_pair(dirLight.name, actor));
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
