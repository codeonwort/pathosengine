#pragma once

#include "badger/types/int_types.h"
#include "pathos/render/postprocessing/anti_aliasing.h"

namespace pathos {

	struct SceneRenderSettings {

		bool isValid() const {
			return (sceneWidth != 0 && sceneHeight != 0);
		}

		uint32 sceneWidth = 0;
		uint32 sceneHeight = 0;
		uint32 frameCounter = 0;
		bool enablePostProcess = true;

	};

}
