#include "scene_desc_parser.h"
#include "pathos/util/log.h"

#include "badger/types/int_types.h"
#include "badger/assertion/assertion.h"

// https://github.com/nlohmann/json
#include <nlohmann/json.hpp>

#define KEY_DIRECTIONAL_LIGHTS    "directionalLight"
#define KEY_POINT_LIGHTS          "pointLight"
#define KEY_STATIC_MESHES         "staticMesh"

namespace pathos {

	template<typename JSONObject = nlohmann::json>
	static vector3 parseVec3(JSONObject& obj) {
		CHECK(obj.size() >= 3);
		vector3 v;
		for (auto i = 0; i < 3; ++i) {
			v[i] = obj[i];
		}
		return v;
	}

	template<typename JSONObject = nlohmann::json>
	static std::string parseName(JSONObject& obj) {
		return obj["name"];
	}

	template<typename JSONObject = nlohmann::json>
	static bool checkMembers(JSONObject& obj, const std::vector<const char*>& members) {
		bool valid = true;
		for (size_t i = 0; i < members.size(); ++i) {
			if (obj.contains(members[i]) == false) {
				LOG(LogError, "Missing property: %s", members[i]);
				valid = false;
			}
		}
		return valid;
	}

	static void parseSky(nlohmann::json& document, SceneDescription& outDesc) {
		int32 hasAtmosphere = (int32)document.contains("skyAtmosphere");
		int32 hasSkybox = (int32)document.contains("skybox");
		int32 hasEquimap = (int32)document.contains("skyEquirectangularMap");
		if (hasAtmosphere + hasSkybox + hasEquimap >= 2) {
			LOG(LogWarning, "Too many sky descriptions. Only one will be activated. (priority: atmosphere -> skybox -> equirectangular map)");
		}

		if (hasAtmosphere) {
			auto sky = document["skyAtmosphere"];
			if (checkMembers(sky, { "name" })) {
				auto name(parseName(sky));

				SceneDescription::SkyAtmosphere desc{ name, true };
				outDesc.skyAtmosphere = std::move(desc);
			}
		}
		if (hasSkybox) {
			auto sky = document["skybox"];
			if (checkMembers(sky, { "name", "flipPreference", "textures", "generateMipmaps" })) {
				auto name(parseName(sky));
				std::string preferenceString(sky["flipPreference"]);
				auto texturePathes = sky["textures"];

				if (texturePathes.size() >= 6) {
					std::vector<std::string> textures(6);
					for (size_t i = 0; i < 6; ++i) {
						textures[i] = texturePathes[i];
					}
					ECubemapImagePreference flipPreference = (preferenceString == "hlsl")
						? ECubemapImagePreference::HLSL
						: ECubemapImagePreference::GLSL;
					const bool mips = sky["generateMipmaps"];

					SceneDescription::Skybox desc{ name, flipPreference, std::move(textures), mips, true };
					outDesc.skybox = std::move(desc);
				} else {
					LOG(LogError, "\"textures\" property should be an array of 6 textures");
				}
			}
		}
		if (hasEquimap) {
			auto sky = document["skyEquirectangularMap"];
			if (checkMembers(sky, { "name", "texture", "hdr" })) {
				std::string name(parseName(sky));
				std::string texture = sky["texture"];

				SceneDescription::SkyEquirectangularMap desc{ name, texture, true };
				outDesc.skyEquimap = std::move(desc);
			}
		}
	}

	static void parseDirLights(nlohmann::json& document, SceneDescription& outDesc) {
		if (!document.contains(KEY_DIRECTIONAL_LIGHTS)) {
			return;
		}

		for (auto& [unused_key, L] : document[KEY_DIRECTIONAL_LIGHTS].items()) {
			if (!checkMembers(L, { "name", "direction", "color", "illuminance" })) {
				continue;
			}

			std::string name(parseName(L));
			vector3 dir = parseVec3(L["direction"]);
			vector3 color = parseVec3(L["color"]);
			float illuminance = L["illuminance"];

			SceneDescription::DirLight desc;
			desc.name = name;
			desc.direction = glm::normalize(dir);
			desc.color = color;
			desc.illuminance = illuminance;

			outDesc.dirLights.emplace_back(desc);
		}
	}

	static void parsePointLights(nlohmann::json& document, SceneDescription& outDesc) {
		if (!document.contains(KEY_POINT_LIGHTS)) {
			return;
		}

		for (auto& [unused_key, L] : document[KEY_POINT_LIGHTS].items()) {
			if (!checkMembers(L, { "name", "location", "intensity",
					"attenuationRadius", "falloffExponent", "castsShadow" })) {
				continue;
			}

			std::string name(parseName(L));
			vector3 loc = parseVec3(L["location"]);
			vector3 intensity = parseVec3(L["intensity"]);
			float attenuationR = L["attenuationRadius"];
			float falloffExp = L["falloffExponent"];
			bool castsShadow = L["castsShadow"];

			SceneDescription::PointLight desc{
				name, loc, intensity, attenuationR, falloffExp, castsShadow
			};
			outDesc.pointLights.emplace_back(desc);
		}
	}

	static void parseStaticMeshes(nlohmann::json& document, SceneDescription& outDesc) {
		if (!document.contains(KEY_STATIC_MESHES)) {
			return;
		}

		for (auto& [unused_key, SM] : document[KEY_STATIC_MESHES].items()) {
			if (!checkMembers(SM, { "name", "location", "rotation", "scale" })) {
				continue;
			}

			std::string name(parseName(SM));
			vector3 loc = parseVec3(SM["location"]);
			vector3 rot = parseVec3(SM["rotation"]);
			vector3 scale = parseVec3(SM["scale"]);

			// #todo-loader: How to support actual assets here?

			SceneDescription::StaticMesh desc{
				name, loc, Rotator(rot.x, rot.y, rot.z), scale
			};
			outDesc.staticMeshes.emplace_back(desc);
		}
	}

	bool SceneDescriptionParser::parse(const std::string& jsonString, SceneDescription& outDesc) {
		nlohmann::json document = nlohmann::json::parse(jsonString);

		// Failed to parse the scene.
		if (document.is_discarded()) {
			return false;
		}

		if (document.contains("name")) {
			outDesc.sceneName = document["name"];
		} else {
			outDesc.sceneName = "<unknown>";
		}

		parseSky(document, outDesc);
		parseDirLights(document, outDesc);
		parsePointLights(document, outDesc);
		parseStaticMeshes(document, outDesc);

		return true;
	}

}
