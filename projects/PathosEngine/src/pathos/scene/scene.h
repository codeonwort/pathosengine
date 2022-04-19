#pragma once

#include "pathos/actor/actor.h"
#include "pathos/material/material_id.h"
#include "pathos/camera/camera.h"

#include "badger/types/matrix_types.h"
#include "gl_core.h"
#include <vector>

namespace pathos {

	// Forward declaration
	enum class SceneProxySource : uint8;
	class SceneProxy;
	class StaticMeshComponent;
	class SkyActor;
	class VolumetricCloudActor;

	// Represents a 3D scene.
	class Scene {
		friend class World;

	public:
		Scene()                        = default;
		Scene(const Scene&)            = delete;
		Scene& operator=(const Scene&) = delete;

		// Generate frame-invariant proxy data.
		SceneProxy* createRenderProxy(SceneProxySource source, uint32 frameNumber, const Camera& camera);

		World* getWorld() const { return owner; }

	public:
		SkyActor* sky = nullptr;
		StaticMeshComponent* godRaySource = nullptr;
		VolumetricCloudActor* cloud = nullptr;

		// IBL
		GLuint irradianceMap = 0;
		GLuint prefilterEnvMap = 0;
		uint32 prefilterEnvMapMipLevels = 0;

	protected:
		World* owner = nullptr;

	};

}
