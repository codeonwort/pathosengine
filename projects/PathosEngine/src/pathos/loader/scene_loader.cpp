#include "scene_loader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"
#include "pathos/actor/world.h"
#include "pathos/light/point_light_actor.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/render/atmosphere.h"
#include "pathos/render/skybox.h"
#include "pathos/render/sky_ansel.h"

#include "badger/system/stopwatch.h"
#include <fstream>
#include <sstream>
#include <array>

#include <FreeImage.h>

namespace pathos {

	bool SceneLoader::loadSceneDescription(
		World* world,
		const char* inFilename,
		ActorBinder& actorBinder)
	{
		LOG(LogDebug, "Loading scene description...: %s", inFilename);

		Stopwatch timer;
		timer.start();

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

		LOG(LogDebug, "Loading done in %f ms", 1000.0f * timer.stop());

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
		// sky
		bool skyBound = false;
		if (sceneDesc.skyAtmosphere.valid) {
			AtmosphereScattering* actor = world->spawnActor<AtmosphereScattering>();

			world->getScene().sky = actor;
			skyBound = true;
			outActorMap.insert(std::make_pair(sceneDesc.skyAtmosphere.name, actor));
		}
		if (sceneDesc.skybox.valid) {
			std::array<const char*, 6> texturePathes;
			for (size_t i = 0; i < 6; ++i) {
				texturePathes[i] = sceneDesc.skybox.textures[i].c_str();
			}
			// #todo: Don't use FIBITMAP* and glObjectLabel directly
			std::array<FIBITMAP*, 6> textureDataArray;
			pathos::loadCubemapImages(texturePathes, sceneDesc.skybox.preference, textureDataArray);
			GLuint cubeTexture = pathos::createCubemapTextureFromBitmap(textureDataArray.data(), sceneDesc.skybox.generateMipmaps);
			glObjectLabel(GL_TEXTURE, cubeTexture, -1, sceneDesc.skybox.name.c_str());

			Skybox* actor = world->spawnActor<Skybox>();
			actor->initialize(cubeTexture);

			if (!skyBound) {
				world->getScene().sky = actor;
				skyBound = true;
			}
			outActorMap.insert(std::make_pair(sceneDesc.skybox.name, actor));
		}
		if (sceneDesc.skyEquimap.valid) {
			GLuint texture = 0;
			if (sceneDesc.skyEquimap.hdr) {
				auto metadata = pathos::loadHDRImage(sceneDesc.skyEquimap.texture.c_str());
				texture = pathos::createTextureFromHDRImage(metadata);
			} else {
				auto metadata = pathos::loadImage(sceneDesc.skyEquimap.texture.c_str());
				texture = pathos::createTextureFromBitmap(metadata, false, true);
			}

			AnselSkyRendering* actor = world->spawnActor<AnselSkyRendering>();
			actor->initialize(texture);

			if (!skyBound) {
				world->getScene().sky = actor;
				skyBound = true;
			}
			outActorMap.insert(std::make_pair(sceneDesc.skyEquimap.name, actor));
		}
		// directional lights
		for (const SceneDescription::DirLight& dirLight : sceneDesc.dirLights) {
			DirectionalLightActor* actor = world->spawnActor<DirectionalLightActor>();
			actor->setLightParameters(dirLight.direction, dirLight.radiance);

			outActorMap.insert(std::make_pair(dirLight.name, actor));
		}
		// point lights
		for (const SceneDescription::PointLight& pLight : sceneDesc.pointLights) {
			PointLightActor* actor = world->spawnActor<PointLightActor>();
			actor->setLightParameters(pLight.radiance, pLight.attenuationRadius,
				pLight.falloffExponent, pLight.castsShadow);
			actor->setActorLocation(pLight.location);

			outActorMap.insert(std::make_pair(pLight.name, actor));
		}
		// static meshes
		for (const SceneDescription::StaticMesh& sm : sceneDesc.staticMeshes) {
			StaticMeshActor* actor = world->spawnActor<StaticMeshActor>();
			actor->setActorLocation(sm.location);
			actor->setActorRotation(sm.rotation);
			actor->setActorScale(sm.scale);

			outActorMap.insert(std::make_pair(sm.name, actor));
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
				*it.second.actor = nullptr;
				LOG(LogError, "Not bound. target actor will be set to NULL: %s", it.first.c_str());
				++countNotBound;
			}
		}
		if (countNotBound > 0) {
			LOG(LogWarning, "%d actors are not bound", countNotBound);
		}
	}

}
