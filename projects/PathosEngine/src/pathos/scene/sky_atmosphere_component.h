#pragma once

#include "pathos/render/scene_proxy.h"
#include "pathos/scene/scene_component.h"
#include "pathos/scene/directional_light_component.h"

#include "badger/types/vector_types.h"

namespace pathos {

	struct SkyAtmosphereProxy : SceneComponentProxy {
		bool bLightingDirty;
	};

	class SkyAtmosphereComponent : public SceneComponent {

	public:
		virtual void createRenderProxy(SceneProxy* scene) override {
			const DirectionalLightComponent* sun = scene->internal_getSunComponent();
			if (sun == nullptr) {
				return;
			}

			bool bLightingDirty = false;
			if (scene->sceneProxySource == SceneProxySource::MainScene) {
				
				float colorDelta = glm::length(sun->illuminance - lastSunIlluminance);
				float cosTheta = glm::dot(sun->direction, lastSunDirectionWS);
				if (colorDelta > 0.1f || cosTheta < 0.99f) {
					lastSunIlluminance = sun->illuminance;
					lastSunDirectionWS = sun->direction;
					bLightingDirty = true;
				}
			}

			SkyAtmosphereProxy* proxy = ALLOC_RENDER_PROXY<SkyAtmosphereProxy>(scene);
			proxy->bLightingDirty = bLightingDirty;

			scene->skyAtmosphere = proxy;
		}

	private:
		vector3 lastSunIlluminance = vector3(-100.0f);
		vector3 lastSunDirectionWS = vector3(0.0f);

	};

}
