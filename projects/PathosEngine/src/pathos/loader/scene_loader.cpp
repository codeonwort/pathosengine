#include "scene_loader.h"

#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"
#include "pathos/scene/world.h"
#include "pathos/scene/point_light_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/skybox_actor.h"
#include "pathos/scene/sky_panorama_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/landscape_actor.h"

#include "badger/system/stopwatch.h"
#include <fstream>
#include <sstream>
#include <array>

namespace pathos {

	bool SceneLoader::loadSceneDescription(World* world, const char* inFilename) {
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

		applyDescription(world, desc, actorMap);

		LOG(LogDebug, "Loading done in %f ms", timer.stop());

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
			auto actor = world->spawnActor<SkyAtmosphereActor>();

			skyBound = true;
			outActorMap.insert(std::make_pair(sceneDesc.skyAtmosphere.name, actor));
		}
		if (sceneDesc.skybox.valid) {
			std::array<const char*, 6> texturePathes;
			for (size_t i = 0; i < 6; ++i) {
				texturePathes[i] = sceneDesc.skybox.textures[i].c_str();
			}
			auto imageBlobs = ImageUtils::loadCubemapImages(texturePathes, sceneDesc.skybox.preference);
			uint32 mipLevels = sceneDesc.skybox.generateMipmaps ? 0 : 1;
			Texture* cubeTexture = ImageUtils::createTextureCubeFromImages(imageBlobs, mipLevels, sceneDesc.skybox.name.c_str());

			auto actor = world->spawnActor<SkyboxActor>();
			actor->setCubemapTexture(cubeTexture);

			skyBound = true;
			outActorMap.insert(std::make_pair(sceneDesc.skybox.name, actor));
		}
		if (sceneDesc.skyEquimap.valid) {
			ImageBlob* blob = ImageUtils::loadImage(sceneDesc.skyEquimap.texture.c_str());
			Texture* texture = ImageUtils::createTexture2DFromImage(blob, 1, false, true, "Texture_Sky");

			auto actor = world->spawnActor<PanoramaSkyActor>();
			actor->setTexture(texture);

			skyBound = true;
			outActorMap.insert(std::make_pair(sceneDesc.skyEquimap.name, actor));
		}
		// directional lights
		for (const SceneDescription::DirLight& dirLight : sceneDesc.dirLights) {
			auto actor = world->spawnActor<DirectionalLightActor>();
			actor->setDirection(dirLight.direction);
			actor->setColor(dirLight.color);
			actor->setIlluminance(dirLight.illuminance);

			outActorMap.insert(std::make_pair(dirLight.name, actor));
		}
		// point lights
		for (const SceneDescription::PointLight& pLight : sceneDesc.pointLights) {
			auto actor = world->spawnActor<PointLightActor>();
			actor->setColor(pLight.color);
			actor->setIntensity(pLight.intensity);
			actor->setAttenuationRadius(pLight.attenuationRadius);
			actor->setFalloffExponent(pLight.falloffExponent);
			actor->setCastsShadow(pLight.castsShadow);
			actor->setActorLocation(pLight.location);

			outActorMap.insert(std::make_pair(pLight.name, actor));
		}
		// static meshes
		for (const SceneDescription::StaticMesh& sm : sceneDesc.staticMeshes) {
			auto actor = world->spawnActor<StaticMeshActor>();
			actor->setActorLocation(sm.location);
			actor->setActorRotation(sm.rotation);
			actor->setActorScale(sm.scale);

			outActorMap.insert(std::make_pair(sm.name, actor));
		}
		// landscapes
		for (const SceneDescription::Landscape& land : sceneDesc.landscapes) {
			auto actor = world->spawnActor<LandscapeActor>();
			actor->setActorLocation(land.location);
			actor->setActorRotation(land.rotation);
			actor->setActorScale(land.scale);

			outActorMap.insert(std::make_pair(land.name, actor));
		}
	}

}
