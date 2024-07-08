#include "world_modelviewer.h"
#include "player_controller.h"

#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/sky_atmosphere_actor.h"
#include "pathos/scene/sky_panorama_actor.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/scene/reflection_probe_actor.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/loader/image_loader.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/gltf_loader.h"
#include "pathos/overlay/brush.h"
#include "pathos/util/file_system.h"
#include "pathos/util/log.h"
#include "pathos/console.h"

#include "badger/system/platform.h"
#include "badger/math/constants.h"
#include "badger/math/minmax.h"


// Initial values
static vector3     SUN_DIRECTION     = vector3(0.0f, -1.0f, -1.0f);
static vector3     SUN_COLOR         = vector3(1.0f);
static float       SUN_ILLUMINANCE   = 1000.0f;
static const char* SKY_PANORAMA_HDRI = "resources/skybox/HDRI/Ridgecrest_Road_Ref.hdr";

class RotationBoard : public DisplayObject2D {

public:
	RotationBoard(float width, float height, float gizmoSize, const wchar_t* txt) {
		board = new pathos::Rectangle(width, height);
		board->setBrush(new SolidColorBrush(0.2f, 0.2f, 0.2f));

		gizmo = new pathos::Rectangle(gizmoSize, gizmoSize);
		gizmo->setBrush(new SolidColorBrush(0.8f, 0.8f, 0.8f));
		gizmo->setX((width - gizmoSize) / 2.0f);
		gizmo->setY((height - gizmoSize) / 2.0f);
		gizmo->bReceivesMouseInput = false;

		board->onMouseDrag = [this](int32 mouseX, int32 mouseY) {
			float u = ((float)mouseX - board->getX()) / board->getWidth();
			float v = ((float)mouseY - board->getY()) / board->getHeight();
			u = badger::clamp(0.0f, u, 1.0f);
			v = badger::clamp(0.0f, v, 1.0f);
			gizmo->setX(mouseX - board->getX() - (gizmo->getWidth() / 2.0f));
			gizmo->setY(mouseY - board->getY() - (gizmo->getHeight() / 2.0f));
			if (onUpdateRotation) {
				onUpdateRotation(u, v);
			}
		};

		label = new pathos::Label(txt);
		label->setX(width + 5.0f);
		label->setColor(vector3(1.0f, 1.0f, 1.0f));

		board->addChild(gizmo);
		addChild(board);
		addChild(label);
	}

	std::function<void(float u, float v)> onUpdateRotation;

private:
	pathos::Rectangle* board = nullptr;
	pathos::Rectangle* gizmo = nullptr;
	pathos::Label*     label = nullptr;

};


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
		{ L"OBJ or glTF", L"*.obj;*.gltf" },
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

	ImageBlob* panoramaImage = ImageUtils::loadImage(SKY_PANORAMA_HDRI);
	panoramaTexture = ImageUtils::createTexture2DFromImage(panoramaImage, 1, false, true, "Texture_Panorama");

	toggleSkyActor();
	toggleProbeGI();

	sunDirection = SUN_DIRECTION;
	sunColor = SUN_COLOR;
	sunIlluminance = SUN_ILLUMINANCE;

	sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(sunDirection);
	sun->setColorAndIlluminance(sunColor, sunIlluminance);

	playerController = spawnActor<PlayerController>();

	modelActor = spawnActor<StaticMeshActor>();

	{
		auto G = new CubeGeometry(vector3(1.0f));
		auto M = pathos::createPBRMaterial(gEngine->getSystemTexture2DWhite());
		M->setConstantParameter("bOverrideMetallic", true);
		M->setConstantParameter("bOverrideRoughness", true);
		M->setConstantParameter("metallicOverride", 1.0f);
		M->setConstantParameter("roughnessOverride", 0.2f);
		dummyBox = spawnActor<StaticMeshActor>();
		dummyBox->setStaticMesh(new Mesh(G, M));

		//auto dummyBox2 = spawnActor<StaticMeshActor>();
		//dummyBox2->setStaticMesh(new Mesh(G, M));
		//dummyBox2->setActorScale(0.8f);
		//dummyBox2->getStaticMesh()->renderInternal = true;
	}

	irradianceVolume = spawnActor<IrradianceVolumeActor>();
	irradianceVolume->initializeVolume(vector3(-2.0f), vector3(2.0f), vector3ui(4, 4, 4));

	// GUI
	{
		label_notice = new Label(L"> ");
		label_notice->setX(130.0f);
		label_notice->setY(15.0f);

		btn_load = new pathos::Button(100.0f, 40.0f, 5.0f, 5.0f);
		btn_load->setX(10.0f);
		btn_load->setY(10.0f);
		btn_load->setBackgroundColor(0.1f, 0.1f, 0.1f);
		btn_load->setText(L"Load model");

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

		btn_toggleSkyActor = new pathos::Button(150.0f, 40.0f, 5.0f, 5.0f);
		btn_toggleSkyActor->setX(10.0f);
		btn_toggleSkyActor->setY(60.0f);
		btn_toggleSkyActor->setBackgroundColor(0.3f, 0.6f, 0.9f);
		btn_toggleSkyActor->setText(L"Toggle sky model");
		btn_toggleSkyActor->onMouseClick = [this](int32 mouseX, int32 mouseY) {
			this->toggleSkyActor();
		};

		btn_toggleProbeGI = new pathos::Button(200.0f, 40.0f, 5.0f, 5.0f);
		btn_toggleProbeGI->setX(10.0f);
		btn_toggleProbeGI->setY(110.0f);
		btn_toggleProbeGI->setBackgroundColor(0.8f, 0.6f, 0.1f);
		btn_toggleProbeGI->setText(L"Toggle probe GI");
		btn_toggleProbeGI->setTextColor(0.1f, 0.1f, 0.1f);
		btn_toggleProbeGI->onMouseClick = [this](int32 mouseX, int32 mouseY) {
			bool bEnabled = this->toggleProbeGI();
			btn_toggleProbeGI->setText(bEnabled ? L"Toggle probe GI (on)" : L"Toggle probe GI (off)");
		};

		board_sunControl = new RotationBoard(100.0f, 100.0f, 8.0f, L"Sun direction");
		board_sunControl->setX(10.0f);
		board_sunControl->setY(160.0f);
		board_sunControl->onUpdateRotation = [this](float u, float v) {
			u = 2.0f * badger::f_PI * u;
			v = badger::f_PI_2 * (1.0f - v);
			sunDirection.x = std::cos(u) * std::cos(v);
			sunDirection.z = std::sin(u) * std::cos(v);
			sunDirection.y = -std::sin(v);
			sun->setDirection(sunDirection);
		};

		board_modelControl = new RotationBoard(100.0f, 100.0f, 8.0f, L"Model rotation");
		board_modelControl->setX(10.0f);
		board_modelControl->setY(270.0f);
		board_modelControl->onUpdateRotation = [this](float u, float v) {
			u = 180.0f * (u - 0.5f);
			v = 360.0f * (v - 0.5f);
			Actor* targetActor = (dummyBox != nullptr) ? dummyBox : modelActor;
			targetActor->setActorRotation(Rotator(u, v, 0.0f));
		};

		auto root = gEngine->getOverlayRoot();
		root->addChild(btn_load);
		root->addChild(label_notice);
		root->addChild(btn_toggleSkyActor);
		root->addChild(btn_toggleProbeGI);
		root->addChild(board_sunControl);
		root->addChild(board_modelControl);
	}
}

void World_ModelViewer::onTick(float deltaSeconds) {
	//
}

void World_ModelViewer::registerConsoleCommands() {
	// Help message
	gConsole->addLine(L"== MODEL VIEWER ======================", false, true);
	gConsole->addLine(L"> Command lists: load_model / sun_color / sun_illuminance / sun_direction", false, true);
	gConsole->addLine(L"> Enter each command without parameters to see help", false, true);
	gConsole->addLine(L"======================================", false, true);

	gEngine->registerConsoleCommand("load_model",
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

	gEngine->registerConsoleCommand("sun_color",
		[this](const std::string& command) {
			float r, g, b;
			int ret = sscanf_s(command.c_str(), "sun_color %f %f %f", &r, &g, &b);
			if (ret != 3) {
				gConsole->addLine(L"Usage: sun_color r g b", false, true);
				wchar_t msg[256];
				swprintf_s(msg, L"Sun color: (%.3f, %.3f, %.3f)", sunColor.r, sunColor.g, sunColor.b);
				gConsole->addLine(msg, false, true);
			} else {
				sunColor.r = glm::clamp(r, 0.0f, 1.0f);
				sunColor.g = glm::clamp(g, 0.0f, 1.0f);
				sunColor.b = glm::clamp(b, 0.0f, 1.0f);
				sun->setColor(sunColor);
			}
		}
	);

	gEngine->registerConsoleCommand("sun_illuminance",
		[this](const std::string& command) {
			float newValue;
			int ret = sscanf_s(command.c_str(), "sun_illuminance %f", &newValue);
			if (ret != 1) {
				gConsole->addLine(L"Usage: sun_illuminance value", false, true);
				wchar_t msg[256];
				swprintf_s(msg, L"Sun illuminance: %.3f lux", sunIlluminance);
				gConsole->addLine(msg, false, true);
			} else {
				sunIlluminance = newValue;
				sun->setIlluminance(sunIlluminance);
			}
		}
	);

	gEngine->registerConsoleCommand("sun_direction",
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

	AABB originalWorldBounds = getActorWorldBounds(modelActor);

	AABB worldBounds = AABB::fromCenterAndHalfSize(originalWorldBounds.getCenter(), 0.9f * originalWorldBounds.getHalfSize());

	// Calculate proper grid size for irradiance volume.
	vector3 probeGridf = worldBounds.getSize() / 0.5f; // per 0.5 meters
	vector3ui probeGrid = vector3ui(std::ceil(probeGridf.x), std::ceil(probeGridf.y), std::ceil(probeGridf.z));
	// Limit the size of the probe grid.
	probeGrid = (glm::max)(probeGrid, vector3ui(2, 2, 2));
	const auto& atlasDesc = getScene().getIrradianceProbeAtlasDesc();
	if (probeGrid.x * probeGrid.y * probeGrid.z > atlasDesc.totalTileCount()) {
		double total = (double)(atlasDesc.totalTileCount());
		uint32 n = (uint32)(std::pow(total, 1.0 / 3.0));
		probeGrid = vector3ui(n, n, n);
	}
	// Recreate irradiance volume.
	if (irradianceVolume != nullptr) {
		irradianceVolume->destroy();
	}
	irradianceVolume = spawnActor<IrradianceVolumeActor>();
	irradianceVolume->initializeVolume(worldBounds.minBounds, worldBounds.maxBounds, probeGrid);

	worldBounds = AABB::fromCenterAndHalfSize(originalWorldBounds.getCenter(), 1.5f * originalWorldBounds.getHalfSize());
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

void World_ModelViewer::toggleSkyActor() {
	const bool bSpawnAtmosphere = (skyAtmosphere == nullptr);

	if (bSpawnAtmosphere) {
		if (panoramaSky != nullptr) {
			panoramaSky->destroy();
			panoramaSky = nullptr;
		}
		skyAtmosphere = spawnActor<SkyAtmosphereActor>();
	} else {
		if (skyAtmosphere != nullptr) {
			skyAtmosphere->destroy();
			skyAtmosphere = nullptr;
		}
		panoramaSky = spawnActor<PanoramaSkyActor>();
		panoramaSky->setTexture(panoramaTexture);
	}
}

bool World_ModelViewer::toggleProbeGI() {
	auto findCVar = [](const char* cvarName) -> ConsoleVariableBase* {
		auto cvar = ConsoleVariableManager::get().find(cvarName);
		if (cvar == nullptr) {
			LOG(LogError, "Can't find cvar: %s", cvarName);
		}
		return cvar;
	};
	auto setCVarInt = [](ConsoleVariableBase* cvar, int32 value) {
		if (cvar != nullptr) {
			static_cast<ConsoleVariable<int32>*>(cvar)->setValue(value);
		}
	};
	auto setCVarFloat = [](ConsoleVariableBase* cvar, float value) {
		if (cvar != nullptr) {
			static_cast<ConsoleVariable<float>*>(cvar)->setValue(value);
		}
	};
	auto cvarUpdateIndirectDiffuse  = findCVar("r.indirectLighting.updateIrradianceProbesPerFrame");
	auto cvarUpdateIndirectSpecular = findCVar("r.indirectLighting.updateReflectionProbesPerFrame");
	auto cvarIndirectLighting       = findCVar("r.indirectLighting");
	if (bEnableProbeGI) {
		setCVarInt(cvarUpdateIndirectDiffuse,  1);
		setCVarInt(cvarUpdateIndirectSpecular, 1);
		setCVarInt(cvarIndirectLighting,       1);
	} else {
		setCVarInt(cvarUpdateIndirectDiffuse,  0);
		setCVarInt(cvarUpdateIndirectSpecular, 0);
		setCVarInt(cvarIndirectLighting,       0);
	}
	bEnableProbeGI = !bEnableProbeGI;
	return !bEnableProbeGI;
}
