#include "scene_desc_parser.h"
#include "pathos/util/log.h"

#include "badger/types/int_types.h"
#include "badger/assertion/assertion.h"
#include <rapidjson/document.h>

namespace pathos {

#define CONTINUE_IF_NOT(it, key) if (0 != strcmp(it->name.GetString(), key)) { continue; }

	template<typename JSONObject>
	static vector3 parseVec3(JSONObject& obj) {
		CHECK(obj.Size() >= 3);
		vector3 v;
		for (rapidjson::SizeType i = 0; i < 3; ++i) {
			v[i] = obj[i].GetFloat();
		}
		return v;
	}

	template<typename JSONObject>
	static std::string parseName(JSONObject& obj) {
		return obj["name"].GetString();
	}

	template<typename JSONObject>
	static bool checkMembers(JSONObject& obj, const std::vector<const char*>& members) {
		bool valid = true;
		for (size_t i = 0; i < members.size(); ++i) {
			if (obj.HasMember(members[i]) == false) {
				LOG(LogError, "Missing property: %s", members[i]);
				valid = false;
			}
		}
		return valid;
	}

	static void parseSky(rapidjson::Document& document, SceneDescription& outDesc) {
		int32 hasAtmosphere = (int32)document.HasMember("skyAtmosphere");
		int32 hasSkybox = (int32)document.HasMember("skybox");
		int32 hasEquimap = (int32)document.HasMember("skyEquirectangular");
		if (hasAtmosphere + hasSkybox + hasEquimap >= 2) {
			LOG(LogWarning, "Too many sky descriptions. Only one will be activated. (priority: atmosphere -> skybox -> equirectangular map)");
		}

		if (hasAtmosphere) {
			auto sky = document["skyAtmosphere"].GetObject();
			auto name(parseName(sky));
			
			SceneDescription::SkyAtmosphere desc{ name, true };
			outDesc.skyAtmosphere = std::move(desc);
		}
		if (hasSkybox) {
			auto sky = document["skybox"].GetObject();
			if (checkMembers(sky, { "flipPreference", "textures", "generateMipmaps" })) {
				auto name(parseName(sky));
				std::string preferenceString(sky["flipPreference"].GetString());
				auto texturePathes = sky["textures"].GetArray();

				if (texturePathes.Size() >= 6) {
					std::vector<std::string> textures(6);
					for (rapidjson::SizeType i = 0; i < 6; ++i) {
						textures[i] = texturePathes[i].GetString();
					}
					ECubemapImagePreference flipPreference = (preferenceString == "hlsl")
						? ECubemapImagePreference::HLSL
						: ECubemapImagePreference::GLSL;
					bool mips = sky["generateMipmaps"].GetBool();

					SceneDescription::Skybox desc{ name, flipPreference, std::move(textures), mips, true };
					outDesc.skybox = std::move(desc);
				}
				if (!outDesc.skybox.valid) {
					LOG(LogError, "skybox has all required members but some are ill-formatted");
				}
			}
		}
		if (hasEquimap) {
			// #todo-scene-loader
		}
	}

	static void parseDirLights(rapidjson::Document& document, SceneDescription& outDesc) {
		for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it) {
			CONTINUE_IF_NOT(it, "directionalLight");

			auto L = it->value.GetObject();
			if (!checkMembers(L, { "name", "direction", "radiance" })) {
				continue;
			}

			auto name(parseName(L));
			auto dir = parseVec3(L["direction"].GetArray());
			auto radiance = parseVec3(L["radiance"].GetArray());

			SceneDescription::DirLight desc;
			desc.name = name;
			desc.direction = glm::normalize(dir);
			desc.radiance = radiance;

			outDesc.dirLights.emplace_back(desc);
		}
	}

	static void parsePointLights(rapidjson::Document& document, SceneDescription& outDesc) {
		for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it) {
			CONTINUE_IF_NOT(it, "pointLight");

			auto L = it->value.GetObject();
			if (!checkMembers(L, { "name", "location", "radiance",
					"attenuationRadius", "falloffExponent", "castsShadow" })) {
				continue;
			}

			auto name(parseName(L));
			auto loc = parseVec3(L["location"].GetArray());
			auto radiance = parseVec3(L["radiance"].GetArray());
			auto attenuationR = L["attenuationRadius"].GetFloat();
			auto falloffExp = L["falloffExponent"].GetFloat();
			auto castsShadow = L["castsShadow"].GetBool();

			SceneDescription::PointLight desc{
				name, loc, radiance, attenuationR, falloffExp, castsShadow
			};
			outDesc.pointLights.emplace_back(desc);
		}
	}

	static void parseStaticMeshes(rapidjson::Document& document, SceneDescription& outDesc) {
		for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it) {
			CONTINUE_IF_NOT(it, "staticMesh");

			auto SM = it->value.GetObject();
			if (!checkMembers(SM, { "name", "location", "rotation", "scale" })) {
				continue;
			}

			auto name(parseName(SM));
			auto loc = parseVec3(SM["location"].GetArray());
			auto rot = parseVec3(SM["rotation"].GetArray());
			auto scale = parseVec3(SM["scale"].GetArray());

			// #todo-scene-loader: How to support actual assets here?

			SceneDescription::StaticMesh desc{
				name, loc, Rotator(rot.x, rot.y, rot.z), scale
			};
			outDesc.staticMeshes.emplace_back(desc);
		}
	}

	bool SceneDescriptionParser::parse(const std::string& jsonSting, SceneDescription& outDesc) {
		rapidjson::Document document;
		document.Parse(jsonSting.c_str());

		if (!document.IsObject()) {
			return false;
		}

		if (document.HasMember("Name")) {
			outDesc.sceneName = document["Name"].GetString();
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
