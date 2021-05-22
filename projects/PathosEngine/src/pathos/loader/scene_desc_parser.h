// Separated from scene_loader due to some nasty include errors for rapidjson.
// ex) rapidjson\document.h(958,76): warning C4003: not enough arguments for function-like macro invocation 'max'

#pragma once

#include "badger/types/vector_types.h"

#include <string>
#include <vector>

namespace pathos {

	struct SceneDescription {
		struct DirLight {
			vector3 direction;
			vector3 radiance;
		};

		std::string sceneName;
		std::vector<DirLight> dirLights;
	};

	class SceneDescriptionParser {
	public:
		bool parse(const std::string& jsonSting, SceneDescription& outDesc);
	};

}
