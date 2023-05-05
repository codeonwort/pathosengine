#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/scene/world.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/button.h"
using namespace pathos;

namespace pathos {
	class Actor;
	class SkyAtmosphereActor;
	class PanoramaSkyActor;
	class DirectionalLightActor;
	class StaticMeshActor;
	class OBJLoader;
	class GLTFLoader;
	class IrradianceVolumeActor;
	class ReflectionProbeActor;
}
class PlayerController;

enum class EModelExt : uint8 { Unknown, Obj, GLTF };
EModelExt findModelFileExtension(const std::string& filepath);

class World_ModelViewer : public World {

public:
	void tryLoadModel(const char* filepath, EModelExt ext);
	void toggleSkyActor();

protected:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void registerConsoleCommands();

	void onLoadOBJ(OBJLoader* loader, uint64 payload);
	void onLoadGLTF(GLTFLoader* loader, uint64 payload);
	void replaceModelActor(Actor* newActor);

private:
	SkyAtmosphereActor* skyAtmosphere = nullptr;
	PanoramaSkyActor* panoramaSky = nullptr;
	GLuint panoramaTexture = 0;

	DirectionalLightActor* sun = nullptr;
	PlayerController* playerController = nullptr;
	Actor* modelActor = nullptr;
	StaticMeshActor* dummyBox = nullptr;

	IrradianceVolumeActor* irradianceVolume = nullptr;
	std::vector<ReflectionProbeActor*> reflectionProbes;

	vector3 sunDirection = vector3(0.0f, -1.0f, -1.0f);
	vector3 sunIlluminance = vector3(10.0f);

	// GUI
	pathos::Button* btn_load = nullptr;
	pathos::Label* label_notice = nullptr;
	pathos::Button* btn_toggleSky = nullptr;
	pathos::Rectangle* board_sunControl = nullptr;

};
