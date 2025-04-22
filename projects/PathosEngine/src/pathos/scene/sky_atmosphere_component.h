#pragma once

#include "pathos/render/scene_proxy.h"
#include "pathos/scene/scene_component.h"
#include "pathos/scene/sky_common.h"
#include "pathos/scene/directional_light_component.h"

#include "badger/types/vector_types.h"

namespace pathos {

	struct SkyAtmosphereProxy : SceneComponentProxy {
		bool bLightingDirty;
		ESkyLightingUpdateMode lightingMode;
		ESkyLightingUpdatePhase lightingPhase;
	};

	class SkyAtmosphereComponent : public SceneComponent {

	public:
		virtual void createRenderProxy(SceneProxy* scene) override {
			const DirectionalLightComponent* sun = scene->internal_getSunComponent();
			if (sun == nullptr || getVisibility() == false) {
				return;
			}

			const bool bMainScene = (scene->sceneProxySource == SceneProxySource::MainScene);
			const ESkyLightingUpdateMode updateMode = getSkyLightingUpdateMode();

			SkyAtmosphereProxy* proxy = ALLOC_RENDER_PROXY<SkyAtmosphereProxy>(scene);
			proxy->bLightingDirty = bMainScene && (updateMode != ESkyLightingUpdateMode::Disabled);
			proxy->lightingMode   = updateMode;
			proxy->lightingPhase  = lightingUpdatePhase;

			if (bMainScene && updateMode == ESkyLightingUpdateMode::Progressive) {
				lightingUpdatePhase = getNextSkyLightingUpdatePhase(lightingUpdatePhase);
			}

			scene->skyAtmosphere = proxy;
		}

	private:
		ESkyLightingUpdatePhase lightingUpdatePhase = (ESkyLightingUpdatePhase)0;

	};

}
