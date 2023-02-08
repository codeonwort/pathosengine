#include "world_modelviewer.h"
#include "player_controller.h"

#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/gltf_loader.h"
#include "pathos/util/file_system.h"
#include "pathos/console.h"

AABB getActorWorldBounds(Actor* actor) {
	AABB bounds{};
	bool bFirst = true;
	for (ActorComponent* comp : actor->getAllComponents()) {
		StaticMeshComponent* smc = dynamic_cast<StaticMeshComponent*>(comp);
		if (smc != nullptr) {
			bounds = bFirst ? smc->getWorldBounds() : (bounds + smc->getWorldBounds());
			bFirst = false;
		}
	}
	return bounds;
}

void World_ModelViewer::onInitialize() {
	registerConsoleCommands();

	getCamera().lookAt(vector3(2.0f, 2.0f, 5.0f), vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f));

	SkyAtmosphereActor* skyAtmosphere = spawnActor<SkyAtmosphereActor>();
	getScene().sky = skyAtmosphere;

	sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(sunDirection);
	sun->setIlluminance(sunIlluminance);

	playerController = spawnActor<PlayerController>();

	modelActor = spawnActor<StaticMeshActor>();

	{
		auto G = new CubeGeometry(vector3(1.0f));
		auto M = pathos::createPBRMaterial(gEngine->getSystemTexture2DWhite());
		dummyBox = spawnActor<StaticMeshActor>();
		dummyBox->setStaticMesh(new Mesh(G, M));
	}

	irradianceVolume = spawnActor<IrradianceVolumeActor>();
	irradianceVolume->initializeVolume(vector3(-2.0f), vector3(2.0f), vector3ui(4, 4, 4));
}

void World_ModelViewer::onTick(float deltaSeconds) {
	//
}

void World_ModelViewer::registerConsoleCommands() {
	// Help message
	gConsole->addLine(L"== MODEL VIEWER ======================", false, true);
	gConsole->addLine(L"> Command lists: load_model / sun_illuminance / sun_direction", false, true);
	gConsole->addLine(L"> Enter each command without parameters to see help", false, true);
	gConsole->addLine(L"======================================", false, true);

	gEngine->registerExec("load_model",
		[this](const std::string& command) {
			auto it = command.find_first_of(' ');
			if (it == std::string::npos) {
				gConsole->addLine(L"> Usage: load_model model_filepath", false, true);
				gConsole->addLine(L"> Supports WavefrontOBJ and Khronos glTF files", false, true);
			} else {
				std::string filepath = command.substr(it + 1);

				size_t extensionIx = filepath.rfind('.');
				EModelExt ext = EModelExt::Unknown;
				if (extensionIx != std::string::npos) {
					std::string s = filepath.substr(extensionIx + 1);
					if (s == "obj") ext = EModelExt::Obj;
					else if (s == "gltf") ext = EModelExt::GLTF;
				}

				if (ext == EModelExt::Unknown) {
					gConsole->addLine(L"> Can't recognize the file format; should be .obj or .gltf", false, true);
				} else {
					wchar_t msg[256];
					swprintf_s(msg, L"Try to load model: '%S'", filepath.c_str());
					gConsole->addLine(msg, false, true);

					tryLoadModel(filepath.c_str(), ext);
				}
			}
		}
	);

	gEngine->registerExec("sun_illuminance",
		[this](const std::string& command) {
			float r, g, b;
			int ret = sscanf_s(command.c_str(), "sun_illuminance %f %f %f", &r, &g, &b);
			if (ret != 3) {
				gConsole->addLine(L"Usage: sun_illuminance r g b", false, true);
				wchar_t msg[256];
				swprintf_s(msg, L"Sun illuminance: (%.3f, %.3f, %.3f) lux", sunIlluminance.r, sunIlluminance.g, sunIlluminance.b);
				gConsole->addLine(msg, false, true);
			} else {
				sunIlluminance.r = r; sunIlluminance.g = g; sunIlluminance.b = b;
				sun->setIlluminance(sunIlluminance);
			}
		}
	);

	gEngine->registerExec("sun_direction",
		[this](const std::string& command) {
			float x, y, z;
			int ret = sscanf_s(command.c_str(), "sun_direction %f %f %f", &x, &y, &z);
			if (ret != 3) {
				gConsole->addLine(L"Usage: sun_direction x y z", false, true);
				wchar_t msg[256];
				swprintf_s(msg, L"Sun direction: (%.3f, %.3f, %.3f)", sunDirection.x, sunDirection.y, sunDirection.z);
				gConsole->addLine(msg, false, true);
			} else {
				sunDirection.x = x; sunDirection.y = y; sunDirection.z = z;
				sun->setDirection(sunDirection);
			}
		}
	);
}

void World_ModelViewer::tryLoadModel(const char* filepath, EModelExt ext) {
	if (ext == EModelExt::Obj) {
		std::string baseDir = pathos::getDirectoryPath(filepath);
		AssetReferenceWavefrontOBJ assetRef(filepath, baseDir.c_str());
		gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRef, this, &World_ModelViewer::onLoadOBJ, 0);
	} else if (ext == EModelExt::GLTF) {
		AssetReferenceGLTF assetRef(filepath);
		gEngine->getAssetStreamer()->enqueueGLTF(assetRef, this, &World_ModelViewer::onLoadGLTF, 0);
	}
}

void World_ModelViewer::onLoadOBJ(OBJLoader* loader, uint64 payload) {
	if (loader->isValid() == false) {
		gConsole->addLine(L"Failed to load the model", false, true);
		return;
	}
	
	Mesh* newModelMesh = loader->craftMeshFromAllShapes(true);
	StaticMeshActor* newActor = spawnActor<StaticMeshActor>();
	newActor->setStaticMesh(newModelMesh);

	replaceModelActor(newActor);
}

void World_ModelViewer::onLoadGLTF(GLTFLoader* loader, uint64 payload) {
	if (loader->isValid() == false) {
		gConsole->addLine(L"Failed to load the model", false, true);
		return;
	}

	Actor* newActor = spawnActor<Actor>();
	loader->attachToActor(newActor);

	replaceModelActor(newActor);
}

void World_ModelViewer::replaceModelActor(Actor* newActor) {
	if (modelActor != nullptr) {
		modelActor->destroy();
	}
	modelActor = newActor;
	modelActor->updateTransformHierarchy();

	if (dummyBox != nullptr) {
		dummyBox->destroy();
		dummyBox = nullptr;
	}

	// #todo-model-viewer: Consider max size of irradiance atlas.
	// Like this: if (required probes > max probes) { setMoreSparseGrid(); }
	// 
	// Recreate irradiance volume.
	AABB worldBounds = getActorWorldBounds(modelActor);
	vector3 probeGridf = worldBounds.getSize() * 0.5f; // per 0.5 meters
	vector3ui probeGrid = vector3ui(::ceilf(probeGridf.x), ::ceilf(probeGridf.y), ::ceilf(probeGridf.z));
	probeGrid = (glm::max)(probeGrid, vector3ui(2, 2, 2));
	if (irradianceVolume != nullptr) {
		irradianceVolume->destroy();
	}
	irradianceVolume = spawnActor<IrradianceVolumeActor>();
	irradianceVolume->initializeVolume(worldBounds.minBounds, worldBounds.maxBounds, probeGrid);

	// #todo-model-viewer: Also auto-place local reflection probes.
	// ...
}
