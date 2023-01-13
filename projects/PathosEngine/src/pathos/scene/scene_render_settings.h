#pragma once

#include "badger/types/int_types.h"
#include "pathos/render/postprocessing/anti_aliasing.h"

namespace pathos {

	class RenderTargetView;

	struct SceneRenderSettings {

		bool isValid() const {
			return (sceneWidth != 0 && sceneHeight != 0);
		}

		uint32 sceneWidth = 0;
		uint32 sceneHeight = 0;
		uint32 frameCounter = 0;
		bool enablePostProcess = true;

		RenderTargetView* finalRenderTarget = nullptr;

		// For probe lighting. Convert and write linear sceneDepth.
		// Note that this texture actually has a color format.
		RenderTargetView* finalDepthTarget = nullptr;

	};

}
