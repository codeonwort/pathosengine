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

		// Notify that sky lighting textures should be cleared. (e.g., due to world transition)
		void invalidateSkyLighting();
		
		// Generate frame-invariant proxy data.
		SceneProxy* createRenderProxy(SceneProxySource source, uint32 frameNumber, const Camera& camera);

		World* getWorld() const { return owner; }

		// -----------------------------------------------------------------------
		// Irradiance Atlas API

		// Irradiance atlas filled by local light probes.
		void initializeIrradianceProbeAtlas();
	
		// @return First tile ID.
		uint32 allocateIrradianceTiles(uint32 numRequiredTiles);

		// Only successful if exactly [firstTileID,lastTileID] was allocated
		// by allocateIrradianceTiles().
		bool freeIrradianceTiles(uint32 firstTileID, uint32 lastTileID);

		void getIrradianceTileTexelOffset(uint32 tileID, uint32& outX, uint32& outY) const;

		// outBounds = (u0, v0, u1, v1)
		void getIrradianceTileBounds(uint32 tileID, vector4& outBounds) const;

		GLuint getIrradianceProbeAtlasTexture() const;
		GLuint getDepthProbeAtlasTexture() const;

	// #todo-godray: Cleanup this mess
	public:
		StaticMeshComponent* godRaySource = nullptr;

	private:
		World* owner = nullptr;

		bool bInvalidateSkyLighting = false;

		uniquePtr<RenderTarget2D> irradianceProbeAtlas;
		uniquePtr<RenderTarget2D> depthProbeAtlas;

		// Save as member to prepare various sizes of atlases per scene.
		uint32 irradianceTileTotalCount = 0;
		uint32 irradianceTileCountX = 0;
		uint32 irradianceTileCountY = 0;
		uint32 irradianceTileSize = 0;

		struct IrradianceTileRange {
			uint32 begin, end; // Both inclusive
			bool operator==(const IrradianceTileRange& other) const {
				return begin == other.begin && end == other.end;
			}
		};
		std::vector<IrradianceTileRange> irradianceTileAllocs;

	};

}
