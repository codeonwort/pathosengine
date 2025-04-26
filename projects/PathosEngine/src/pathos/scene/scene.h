#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/render/image_based_lighting.h"
#include "pathos/material/material_id.h"
#include "pathos/scene/actor.h"
#include "pathos/scene/camera.h"
#include "pathos/smart_pointer.h"

#include "badger/types/matrix_types.h"
#include <vector>

namespace pathos {

	// Forward declaration
	enum class SceneProxySource : uint8;
	class SceneProxy;
	class Fence;
	class StaticMeshComponent;
	class SkyActor;
	class VolumetricCloudActor;
	class RenderTarget2D;
	class Buffer;

	// Represents a 3D scene.
	class Scene final {
		friend class World;
		friend class IrradianceVolumeActor;
		friend class ReflectionProbeActor;

	public:
		Scene();
		~Scene();

		Scene(const Scene&)            = delete;
		Scene& operator=(const Scene&) = delete;
		
		// Generate frame-invariant proxy data.
		SceneProxy* createRenderProxy(
			SceneProxySource source,
			uint32 frameNumber,
			const Camera& camera,
			Fence* fence = nullptr,
			uint64 fenceValue = 0);

		inline World* getWorld() const { return owner; }

		// -----------------------------------------------------------------------
		// Light System

		// Notify that sky lighting textures should be cleared. (e.g., due to world transition)
		void invalidateSkyLighting();

		inline LightProbeScene& getLightProbeScene() { return lightProbeScene; }

		void initializeIrradianceProbeAtlasDesc(const IrradianceProbeAtlasDesc& desc);

		void updateLightProbes();

	private:
		void registerIrradianceVolume(IrradianceVolumeActor* actor);
		void unregisterIrradianceVolume(IrradianceVolumeActor* actor);
		void registerReflectionProbe(ReflectionProbeActor* actor);
		void unregisterReflectionProbe(ReflectionProbeActor* actor);

	// #todo-godray: Cleanup this mess
	public:
		StaticMeshComponent* godRaySource    = nullptr;
		vector3              godRayColor     = vector3(1.0f, 0.5f, 0.0f);
		float                godRayIntensity = 1.0f;

	private:
		World*                              owner = nullptr;
		std::vector<ReflectionProbeActor*>  reflectionProbes; // Actors spawned in the owner world
		std::vector<IrradianceVolumeActor*> irradianceVolumes; // Actors spawned in the owner world
		LightProbeScene                     lightProbeScene;
		bool                                bInvalidateSkyLighting = false;
	};

}
