#include "world_modelviewer.h"
#include "player_controller.h"

#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/scene/reflection_probe_actor.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/gltf_loader.h"
#include "pathos/util/file_system.h"
#include "pathos/console.h"

#include "pathos/overlay/brush.h"
#include "pathos/util/log.h"

#include "badger/system/platform.h"

#if PLATFORM_WINDOWS
// https://learn.microsoft.com/en-us/windows/win32/learnwin32/example--the-open-dialog-box
#include <Windows.h>
#include <ShObjIdl.h>
std::string browseModelFile() {
	HRESULT hr = CoInitializeEx(
		NULL,
		COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) return "";
	IFileOpenDialog* pFileOpen;
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
	if (FAILED(hr)) return "";

	COMDLG_FILTERSPEC filters[] = {
		{ L"Wavefront OBj", L"*.obj" }, { L"Khronos glTF", L"*.gltf" },
	};
	hr = pFileOpen->SetFileTypes(_countof(filters), filters);
	if (FAILED(hr)) return "";

	hr = pFileOpen->Show(NULL);
	if (FAILED(hr)) return "";
	IShellItem* pItem;
	hr = pFileOpen->GetResult(&pItem);
	if (FAILED(hr)) return "";
	PWSTR filePathW;
	hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePathW);
	if (FAILED(hr)) return "";
	std::string filePathA;
	pathos::WCHAR_TO_MBCS(filePathW, filePathA);
	pItem->Release();
	pFileOpen->Release();
	CoUninitialize();
	return filePathA;
}
#else
std::string browseModelFile() {
	return "";
}
#endif

EModelExt findModelFileExtension(const std::string& filepath) {
	size_t extensionIx = filepath.rfind('.');
	EModelExt ext = EModelExt::Unknown;
	if (extensionIx != std::string::npos) {
		std::string s = filepath.substr(extensionIx + 1);
		if (s == "obj") ext = EModelExt::Obj;
		else if (s == "gltf") ext = EModelExt::GLTF;
	}
	return ext;
}

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

	// GUI
	{
		Label* label_load = new Label(L"Load model");
		label_load->setX(5.0f);
		label_load->setY(5.0f);

		label_notice = new Label(L"> ");
		label_notice->setX(130.0f);
		label_notice->setY(15.0f);

		btn_load = new pathos::Rectangle(100.0f, 40.0f);
		btn_load->setX(10.0f);
		btn_load->setY(10.0f);
		btn_load->setBrush(new SolidColorBrush(0.1f, 0.1f, 0.1f));
		btn_load->addChild(label_load);

		auto pNotice = label_notice;
		btn_load->onMouseClick = [pNotice, this](int32 mouseX, int32 mouseY) {
			std::string filepath = browseModelFile();

			if (filepath.size() > 0) {
				std::wstring filepathW;
				pathos::MBCS_TO_WCHAR(filepath, filepathW);
				pNotice->setText(filepathW.c_str());

				EModelExt ext = findModelFileExtension(filepath);
				tryLoadModel(filepath.c_str(), ext);
			} else {
				pNotice->setText(L"Select a file to load");
			}
		};

		gEngine->getOverlayRoot()->addChild(btn_load);
		gEngine->getOverlayRoot()->addChild(label_notice);
	}
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

	AABB worldBounds = getActorWorldBounds(modelActor);

	// Calculate proper grid size for irradiance volume.
	vector3 probeGridf = worldBounds.getSize() / 0.5f; // per 0.5 meters
	vector3ui probeGrid = vector3ui(std::ceil(probeGridf.x), std::ceil(probeGridf.y), std::ceil(probeGridf.z));
	// Limit the size of the probe grid.
	probeGrid = (glm::max)(probeGrid, vector3ui(2, 2, 2));
	if (probeGrid.x * probeGrid.y * probeGrid.z > pathos::irradianceProbeTileCountX * pathos::irradianceProbeTileCountY) {
		double total = (double)(pathos::irradianceProbeTileCountX * pathos::irradianceProbeTileCountY);
		uint32 n = (uint32)(std::pow(total, 1.0 / 3.0));
		probeGrid = vector3ui(n, n, n);
	}
	// Recreate irradiance volume.
	if (irradianceVolume != nullptr) {
		irradianceVolume->destroy();
	}
	irradianceVolume = spawnActor<IrradianceVolumeActor>();
	irradianceVolume->initializeVolume(worldBounds.minBounds, worldBounds.maxBounds, probeGrid);

	worldBounds = AABB::fromCenterAndHalfSize(worldBounds.getCenter(), worldBounds.getHalfSize() * 1.5f);
	vector3 uvw = worldBounds.getSize() / 10.0f; // per 10.0 meters
	vector3ui reflectionProbeCount = vector3ui(std::ceil(uvw.x), std::ceil(uvw.y), std::ceil(uvw.z));
	reflectionProbeCount = (glm::max)(reflectionProbeCount, vector3ui(2, 2, 2));
	for (ReflectionProbeActor* oldProbe : reflectionProbes) {
		oldProbe->destroy();
	}
	reflectionProbes.clear();
	bool bNeedReflectionProbeAtCenter = true;
	for (uint32 xi = 0; xi < reflectionProbeCount.x; ++xi) {
		for (uint32 yi = 0; yi < reflectionProbeCount.y; ++yi) {
			for (uint32 zi = 0; zi < reflectionProbeCount.z; ++zi) {
				ReflectionProbeActor* probe = spawnActor<ReflectionProbeActor>();
				vector3 ratio;
				ratio.x = ((float)xi / (reflectionProbeCount.x - 1));
				ratio.y = ((float)yi / (reflectionProbeCount.y - 1));
				ratio.z = ((float)zi / (reflectionProbeCount.z - 1));
				vector3 pos = worldBounds.minBounds + ratio * (worldBounds.maxBounds - worldBounds.minBounds);
				probe->setActorLocation(pos);
				reflectionProbes.push_back(probe);
				if (glm::length(pos) < 0.5f) {
					bNeedReflectionProbeAtCenter = false;
				}
			}
		}
	}
	if (bNeedReflectionProbeAtCenter) {
		ReflectionProbeActor* reflectionProbe0 = spawnActor<ReflectionProbeActor>();
		reflectionProbe0->setActorLocation(worldBounds.getCenter());
		reflectionProbes.push_back(reflectionProbe0);
	}
}
