// Load a scene from a scene description file.

// #note-scene: Actually pathos::Scene does not corresponds with scene description
// as actors are owned by pathos::World. The scene description is mainly for a static scene
// including meshes, lights, and sky, but more components might be put.

#pragma once

#include "scene_desc_parser.h"
#include "pathos/scene/actor.h"

#include <vector>
#include <map>

namespace pathos {

	class World;
	//class Actor;

	class ActorBinder {
		friend class SceneLoader;
		struct Info {
			void** actor;
			bool bound;
		};
	public:
		template<typename ActorType>
		void addBinding(const std::string& name, ActorType** actor) {
			static_assert(std::is_base_of<Actor, ActorType>::value, "Target type is not derived from pathos::Actor");
			CHECKF(bindings.find(name) == bindings.end(), "Duplicate name");

			void** actorBase = reinterpret_cast<void**>(actor);
			bindings.insert(std::make_pair(name, Info{ actorBase, false }));
		}
	private:
		std::map<std::string, Info> bindings;
	};

	class SceneLoader {
	public:
		bool loadSceneDescription(
			World* world,
			const char* inFilename,
			ActorBinder& actorBinder);

	private:
		using ActorMap = std::map<std::string, Actor*>;

		bool loadJSON(const char* inFilename, std::string& outJSON);
		bool parseJSON(const std::string& inJSON, SceneDescription& outDesc);
		void applyDescription(World* world, const SceneDescription& desc, ActorMap& outActorMap);
		void bindActors(SceneDescription& desc, const ActorMap& actorMap, ActorBinder& binder);
	};

}
