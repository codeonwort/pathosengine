#include "world_modelviewer.h"
#include "player_controller.h"

#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/loader/objloader.h"
#include "pathos/util/file_system.h"
#include "pathos/console.h"

void World_ModelViewer::onInitialize() {
	registerConsoleCommands();

	getCamera().lookAt(vector3(2.0f, 2.0f, 5.0f), vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f));

	playerController = spawnActor<PlayerController>();

	auto G = new CubeGeometry(vector3(1.0f));
	auto M = pathos::createPBRMaterial(gEngine->getSystemTexture2DBlue());

	StaticMeshActor* dummyBox = spawnActor<StaticMeshActor>();
	dummyBox->setStaticMesh(new Mesh(G, M));

	DirectionalLightActor* sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(vector3(0.0f, -1.0f, -1.0f));
	sun->setIlluminance(vector3(10.0f));

	modelActor = spawnActor<StaticMeshActor>();
}

void World_ModelViewer::onTick(float deltaSeconds) {
	//
}

void World_ModelViewer::registerConsoleCommands() {
	// Help message
	gConsole->addLine(L"== MODEL VIEWER ======================", false, true);
	gConsole->addLine(L"Command list: load_model", false, true);
	gConsole->addLine(L"======================================", false, true);

	gEngine->registerExec("load_model",
		[this](const std::string& command) {
			auto it = command.find_first_of(' ');
			if (it == std::string::npos) {
				gConsole->addLine(L"Usage: load_model <model_filepath>", false, true);
			} else {
				std::string filepath = command.substr(it + 1);

				wchar_t msg[256];
				swprintf_s(msg, L"Try to load model: '%S'", filepath.c_str());
				gConsole->addLine(msg, false, true);

				tryLoadModel(filepath.c_str());
			}
		}
	);
}

void World_ModelViewer::tryLoadModel(const char* filepath) {
	std::string baseDir = pathos::getDirectoryPath(filepath);
	AssetReferenceWavefrontOBJ assetRef(filepath, baseDir.c_str());
	gEngine->getAssetStreamer()->enqueueWavefrontOBJ(assetRef, this, &World_ModelViewer::onLoadOBJ, 0);
}

void World_ModelViewer::onLoadOBJ(OBJLoader* loader, uint64 payload) {
	if (loader->isValid() == false) {
		gConsole->addLine(L"Failed to load the model", false, true);
		return;
	}
	
	Mesh* newModelMesh = loader->craftMeshFromAllShapes(true);
	Mesh* oldMesh = modelActor->getStaticMesh();
	if (oldMesh != nullptr) {
		delete oldMesh;
	}
	modelActor->setStaticMesh(newModelMesh);
}
