// Load a scene from a scene description file.

// #note-scene: Actually pathos::Scene does not corresponds with scene description
// as actors are owned by pathos::World. The scene description is mainly for a static scene
// including meshes, lights, and sky, but more components might be put.

#pragma once

#include "scene_desc_parser.h"
#include "pathos/scene/actor.h"
#include "pathos/smart_pointer.h"

#include <vector>
#include <map>

namespace pathos {

	class World;

	class SceneLoader {

	public:
		bool loadSceneDescription(World* world, const char* inFilename);

		template<typename T>
		void bindActor(const std::string& name, actorPtr<T>* targetActor) {
			auto it = actorMap.find(name);
			if (it != actorMap.end()) {
				*targetActor = dynamicCastActor<T>(it->second);
			} else {
				CHECK_NO_ENTRY();
			}
		}

	private:
		using ActorMap = std::map<std::string, actorPtr<Actor>>;

		bool loadJSON(const char* inFilename, std::string& outJSON);
		bool parseJSON(const std::string& inJSON, SceneDescription& outDesc);
		void applyDescription(World* world, const SceneDescription& desc, ActorMap& outActorMap);

	private:
		ActorMap actorMap;
	};

}
