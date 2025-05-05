#pragma once

// This header file contains common definitions for Image Based Lighting.

#include "pathos/render/render_target.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include <vector>
#include <list>

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
		uint32  cubemapIndex;
		uint32  _pad0;
		uint32  _pad1;
		uint32  _pad2;
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

	class ReflectionProbeAllocator {

	public:
		void initialize(uint32 inMaxCount) {
			maxCount = inMaxCount;

			for (uint32 i = 0; i < maxCount; ++i) {
				cubemapIndices.push_back(i);
			}
		}

		inline bool canAllocate() const { return cubemapIndices.size() > 0; }

		inline uint32 allocateIndex() {
			CHECK(canAllocate());
			uint32 ix = cubemapIndices.front();
			cubemapIndices.pop_front();
			return ix;
		}

		inline void releaseIndex(uint32 index) {
			CHECK(0 <= index && index < maxCount);
			auto it = cubemapIndices.begin();
			while (it != cubemapIndices.end()) {
				CHECK(*it != index);
				if (index < *it) break;
				++it;
			}
			cubemapIndices.insert(it, index);
		}

		inline uint32 getMaxCount() const { return maxCount; }

	private:
		uint32 maxCount = 0;
		std::list<uint32> cubemapIndices;
	};

	// Manages light probe data in a Scene.
	class LightProbeScene {
	public:
		LightProbeScene();

		void initializeIrradianceProbeAtlasDesc(const IrradianceProbeAtlasDesc& desc);
		void createGPUResources();

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

		Buffer* getIrradianceSHBuffer() const;

		uint32 allocateReflectionProbe();
		void releaseReflectionProbe(uint32 index);
		Texture* getReflectionProbeArrayTexture() const;

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
		uniquePtr<Buffer>                irradianceSHBuffer;

		ReflectionProbeAllocator         reflectionProbeAllocator;
		uniquePtr<Texture>               reflectionProbeArrayTexture;
		uniquePtr<Buffer>                reflectionProbeBuffer;
	};

}
