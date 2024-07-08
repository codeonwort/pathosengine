#pragma once

// This header file contains common definitions for Image Based Lighting.

#include "pathos/render/render_target.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"

namespace pathos {

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

}
