#pragma once

// This header file contains common definitions for Image Based Lighting.

#include "pathos/render/render_target.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"

namespace pathos {

	class Buffer;
	class RenderTarget2D;
	class SceneProxy;

	const RenderTargetFormat IRRADIANCE_PROBE_FORMAT = RenderTargetFormat::RGBA16F;
	const RenderTargetFormat DEPTH_PROBE_FORMAT      = RenderTargetFormat::R16F;

	// Shaders expect a SSBO containing an array of this struct.
	struct IrradianceVolumeInfo {
		vector3   minBounds;
		uint32    firstTileID;   // ID of the first tile in irradiance probe atlas
		vector3   maxBounds;
		uint32    numProbes;     // Total probes in this volume
		vector3ui gridSize;
		float     captureRadius; // Max draw distance of each probe
	};

	// Shaders expect a SSBO containing an array of this struct.
	struct ReflectionProbeInfo {
		vector3 positionWS;    // Center of the reflection probe
		float   captureRadius; // Max draw distance of this probe
	};

	struct IrradianceProbeAtlasDesc {
		static constexpr uint32 INVALID_TILE_ID = 0xffffffff;

		uint32 tileSize   = 0;
		uint32 tileCountX = 0;
		uint32 tileCountY = 0;

		uint32 totalTileCount() const { return tileCountX * tileCountY; }
	};

	struct IrradianceProbeID {
		uint32 firstTileID = IrradianceProbeAtlasDesc::INVALID_TILE_ID;
		uint32 firstShIndex = IrradianceProbeAtlasDesc::INVALID_TILE_ID;

		inline bool isValid() const { return firstTileID != IrradianceProbeAtlasDesc::INVALID_TILE_ID; }
	};

	// Manages light probe data in a Scene.
	class LightProbeScene {
	public:
		LightProbeScene();

		void initializeIrradianceProbeAtlasDesc(const IrradianceProbeAtlasDesc& desc);
		void createIrradianceProbeAtlas();

		// @return First tile ID.
		IrradianceProbeID allocateIrradianceTiles(uint32 numRequiredTiles);

		// Only successful if exactly [firstTileID,lastTileID] was allocated
		// by allocateIrradianceTiles().
		bool freeIrradianceTiles(uint32 firstTileID, uint32 lastTileID);

		void getIrradianceTileTexelOffset(uint32 tileID, uint32& outX, uint32& outY) const;

		// outBounds = (u0, v0, u1, v1)
		void getIrradianceTileBounds(uint32 tileID, vector4& outBounds) const;

		GLuint getIrradianceProbeAtlasTexture() const;
		GLuint getDepthProbeAtlasTexture() const;
		inline const IrradianceProbeAtlasDesc& getIrradianceProbeAtlasDesc() const { return irradianceProbeAtlasDesc; }

		void createSceneProxy(SceneProxy* sceneProxy, bool isLightProbeRendering);

	private:
		struct IrradianceTileRange {
			uint32 begin, end; // Both inclusive
			bool operator==(const IrradianceTileRange& other) const {
				return begin == other.begin && end == other.end;
			}
		};

		std::vector<IrradianceTileRange> irradianceTileAllocs;
		IrradianceProbeAtlasDesc         irradianceProbeAtlasDesc;
		uniquePtr<RenderTarget2D>        irradianceProbeAtlas;
		uniquePtr<RenderTarget2D>        depthProbeAtlas;
		uniquePtr<Buffer>                irradianceVolumeBuffer;
		uniquePtr<Buffer>                reflectionProbeBuffer;
		uniquePtr<Buffer>                irradianceSHBuffer;
	};

}
