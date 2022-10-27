#pragma once

#include "pathos/render/scene_proxy.h"
#include "pathos/actor/scene_component.h"

namespace pathos {

	struct SkyAtmosphereProxy : SceneComponentProxy {
		// No parameters for now
		float dummy = 0.0f;
	};

	class SkyAtmosphereComponent : public SceneComponent {

	public:
		virtual void createRenderProxy(SceneProxy* scene) override {
			SkyAtmosphereProxy* proxy = ALLOC_RENDER_PROXY<SkyAtmosphereProxy>(scene);
			proxy->dummy = 0.0f;

			scene->skyAtmosphere = proxy;
		}

	};

}
