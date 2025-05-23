#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/scene/world.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/button.h"
#include "pathos/smart_pointer.h"
using namespace pathos;

namespace pathos {
	class Texture;
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
class RotationBoard;

enum class EModelExt : uint8 { Unknown, Obj, GLTF };
EModelExt findModelFileExtension(const std::string& filepath);

class World_ModelViewer : public World {

public:
	void tryLoadModel(const char* filepath, EModelExt ext);
	void toggleSkyActor();
	bool toggleProbeGI();

protected:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void registerConsoleCommands();

	void onLoadOBJ(OBJLoader* loader, uint64 payload);
	void onLoadGLTF(GLTFLoader* loader, uint64 payload);
	void replaceModelActor(sharedPtr<Actor> newActor);

private:
	bool bEnableProbeGI = true;

	actorPtr<SkyAtmosphereActor> skyAtmosphere;
	actorPtr<PanoramaSkyActor> panoramaSky;
	Texture* panoramaTexture = nullptr;

	actorPtr<DirectionalLightActor> sun;
	actorPtr<PlayerController> playerController;
	actorPtr<Actor> modelActor;
	actorPtr<StaticMeshActor> dummyBox;

	actorPtr<IrradianceVolumeActor> irradianceVolume;
	actorPtrList<ReflectionProbeActor> reflectionProbes;

	vector3 sunDirection;
	vector3 sunColor;
	float sunIlluminance;

	// GUI
	pathos::Button* btn_load            = nullptr;
	pathos::Label*  label_notice        = nullptr;
	pathos::Button* btn_toggleSkyActor  = nullptr;
	pathos::Button* btn_toggleProbeGI   = nullptr;
	RotationBoard*  board_sunControl    = nullptr;
	RotationBoard*  board_modelControl  = nullptr;

};
