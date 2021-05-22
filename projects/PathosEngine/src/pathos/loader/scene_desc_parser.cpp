#include "scene_desc_parser.h"

#include "badger/assertion/assertion.h"
#include <rapidjson/document.h>

namespace pathos {

	template<typename JSONObject>
	static vector3 parseVec3(JSONObject& obj) {
		CHECK(obj.Size() >= 3);
		vector3 v;
		for (rapidjson::SizeType i = 0; i < 3; ++i) {
			v[i] = obj[i].GetFloat();
		}
		return v;
	}

	static void parseDirLights(rapidjson::Document& document, SceneDescription& outDesc) {
		for (auto it = document.FindMember("directionalLight"); it != document.MemberEnd(); ++it) {
			auto L = it->value.GetObject();
			if (!L.HasMember("direction") || !L.HasMember("radiance")) {
				continue;
			}

			auto dir = parseVec3(L["direction"].GetArray());
			auto radiance = parseVec3(L["radiance"].GetArray());

			SceneDescription::DirLight desc;
			desc.direction = glm::normalize(dir);
			desc.radiance = radiance;

			outDesc.dirLights.emplace_back(desc);
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

		parseDirLights(document, outDesc);
		// #todo-scene-loader: point lights
		// #todo-scene-loader: static meshes (primitive or 3d model files)

		return true;
	}

}
