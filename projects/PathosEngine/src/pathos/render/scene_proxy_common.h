#pragma once

#include "pathos/render/image_based_lighting.h"
#include "pathos/scene/camera.h"

namespace pathos {

	class Fence;
	class Texture;

	enum class SceneProxySource : uint8 {
		MainScene         = 0,
		SceneCapture      = 1,
		RadianceCapture   = 2,
		IrradianceCapture = 3, // In fact same as RadianceCapture but render to smaller RT. Filtered for irradiance caching.
	};

	inline const char* getSceneProxySourceString(SceneProxySource source) {
		const char* str[] = {
			"MainScene",
			"SceneCapture",
			"RadianceCapture",
			"IrradianceCapture",
		};
		return str[(uint8)source];
	}

	inline bool isLightProbeRendering(SceneProxySource source) {
		return source == SceneProxySource::RadianceCapture || source == SceneProxySource::IrradianceCapture;
	}

	struct SceneProxyCreateParams {
		SceneProxySource proxySource;
		uint32           frameNumber;
		const Camera&    camera;
		Fence*           fence                            = nullptr;
		uint64           fenceValue                       = 0;
		// For light probe SH
		uint32           lightProbeShIndex                = IrradianceProbeAtlasDesc::INVALID_TILE_ID;
		Texture*         lightProbeColorCubemap           = nullptr;
		Texture*         lightProbeDepthCubemap           = nullptr;
		vector4ui        lightProbeDepthAtlasCoordAndSize = vector4ui(0, 0, 0, 0);
	};

}
