#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/scene/actor.h"
#include "pathos/material/material_id.h"
#include "pathos/scene/camera.h"
#include "pathos/smart_pointer.h"

#include "badger/types/matrix_types.h"
#include <vector>

namespace pathos {

	// Forward declaration
	enum class SceneProxySource : uint8;
	class SceneProxy;
	class StaticMeshComponent;
	class SkyActor;
	class VolumetricCloudActor;
	class RenderTarget2D;

	// Represents a 3D scene.
	class Scene final {
		friend class World;

	public:
		Scene();
		~Scene();

		Scene(const Scene&)            = delete;
		Scene& operator=(const Scene&) = delete;

		// Irradiance atlas filled by local light probes.
		void initializeIrradianceProbeAtlas();
		bool allocateIrradianceTile(uint32& outTileID, uint32& outViewportX, uint32& outViewportY);
		bool freeIrradianceTile(uint32 tileID);
		// outBounds = (u0, v0, u1, v1)
		void getIrradianceTileBounds(uint32 tileID, vector4& outBounds);
		GLuint getIrradianceAtlasTexture() const;

		// Generate frame-invariant proxy data.
		SceneProxy* createRenderProxy(SceneProxySource source, uint32 frameNumber, const Camera& camera);

		World* getWorld() const { return owner; }

	public:
		SkyActor* sky = nullptr;
		StaticMeshComponent* godRaySource = nullptr;
		VolumetricCloudActor* cloud = nullptr;

		// Sky IBL
		GLuint skyIrradianceMap = 0;
		GLuint skyPrefilterEnvMap = 0;
		uint32 skyPrefilterEnvMapMipLevels = 0;

	private:
		void getIrradianceTileOffset(uint32 tileID, uint32& outX, uint32& outY) const;

		World* owner = nullptr;

		uniquePtr<RenderTarget2D> irradianceProbeAtlas;
		std::vector<bool> irradianceTileAllocs;

	};

}
