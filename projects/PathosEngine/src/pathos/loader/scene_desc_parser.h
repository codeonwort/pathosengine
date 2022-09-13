// Separated from scene_loader due to some nasty include errors for rapidjson.
// ex) rapidjson\document.h(958,76): warning C4003: not enough arguments for function-like macro invocation 'max'

#pragma once

#include "pathos/loader/imageloader.h"

#include "badger/types/vector_types.h"
#include "badger/math/rotator.h"

#include <string>
#include <vector>

namespace pathos {

	struct SceneDescription {
		struct SkyAtmosphere {
			std::string name;
			bool valid = false;
		};
		struct Skybox {
			std::string name;
			ECubemapImagePreference preference;
			std::vector<std::string> textures;
			bool generateMipmaps;
			bool valid = false;
		};
		struct SkyEquirectangularMap {
			std::string name;
			std::string texture;
			bool hdr;
			bool valid = false;
		};
		struct DirLight {
			std::string name;
			vector3 direction;
			vector3 radiance;
		};
		struct PointLight {
			std::string name;
			vector3 location;
			vector3 radiance;
			float attenuationRadius;
			float falloffExponent;
			bool castsShadow;
		};
		struct StaticMesh {
			std::string name;
			vector3 location;
			Rotator rotation;
			vector3 scale;
		};

		std::string sceneName;
		SkyAtmosphere skyAtmosphere;
		Skybox skybox;
		SkyEquirectangularMap skyEquimap;
		std::vector<DirLight> dirLights;
		std::vector<PointLight> pointLights;
		std::vector<StaticMesh> staticMeshes;
	};

	class SceneDescriptionParser {
	public:
		bool parse(const std::string& jsonSting, SceneDescription& outDesc);
	};

}
