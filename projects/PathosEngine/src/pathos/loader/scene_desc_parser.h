#pragma once

#include "pathos/loader/image_loader.h"

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
			bool valid = false;
		};
		struct DirLight {
			std::string name;
			vector3 direction;
			vector3 color;
			float illuminance;
		};
		struct PointLight {
			std::string name;
			vector3 location;
			vector3 color;
			float intensity;
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
