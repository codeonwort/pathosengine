#pragma once

#include "scene.h"
#include "pathos/camera/camera.h"

namespace pathos {

	class SkyRendering {
		
	public:
		SkyRendering()          = default;
		virtual ~SkyRendering() = default;

		virtual void render(const Scene* scene, const Camera* camera) = 0;

	};

}