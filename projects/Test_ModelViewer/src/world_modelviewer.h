#pragma once

#include "pathos/scene/world.h"
using namespace pathos;

namespace pathos {
	class Actor;
	class DirectionalLightActor;
	class StaticMeshActor;
	class OBJLoader;
	class GLTFLoader;
	class IrradianceVolumeActor;
	class ReflectionProbeActor;
}
class PlayerController;

class World_ModelViewer : public World {

	enum class EModelExt : uint8 { Unknown, Obj, GLTF };

protected:
	virtual void onInitialize() override;
	virtual void onTick(float deltaSeconds) override;

private:
	void registerConsoleCommands();
	void tryLoadModel(const char* filepath, EModelExt ext);

	void onLoadOBJ(OBJLoader* loader, uint64 payload);
	void onLoadGLTF(GLTFLoader* loader, uint64 payload);
	void replaceModelActor(Actor* newActor);

private:
	DirectionalLightActor* sun = nullptr;
	PlayerController* playerController = nullptr;
	Actor* modelActor = nullptr;
	StaticMeshActor* dummyBox = nullptr;

	IrradianceVolumeActor* irradianceVolume = nullptr;
	std::vector<ReflectionProbeActor*> reflectionProbes;

	vector3 sunDirection = vector3(0.0f, -1.0f, -1.0f);
	vector3 sunIlluminance = vector3(10.0f);

};
