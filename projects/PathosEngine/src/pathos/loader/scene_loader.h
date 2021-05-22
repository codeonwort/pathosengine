// Load a scene from a scene description file.

// #note-scene: Actually pathos::Scene does not corresponds with scene description
// as actors are owned by pathos::World. The scene description is mainly for a static scene
// including meshes, lights, and sky, but more components might be put.

#pragma once

#include "scene_desc_parser.h"

namespace pathos {

	class World;

	class SceneLoader {
	public:
		bool loadSceneDescription(World* world, const char* inFilename);

		// #todo-scene-loader: Support unload?
		//void unload();

	private:
		bool loadJSON(const char* inFilename, std::string& outJSON);
		bool parseJSON(const std::string& inJSON, SceneDescription& outDesc);
		void applyDescription(World* world, const SceneDescription& desc);
	};

}
