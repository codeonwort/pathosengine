#pragma once

#include "pathos/scene/scene_component.h"

namespace pathos {

	struct DirectionalLightProxy : public SceneComponentProxy {
		vector3 wsDirection;
		float   padding0;
		vector3 illuminance;
		float   padding1;
		vector3 vsDirection;
		float   padding2;
	};

	class DirectionalLightComponent : public SceneComponent {

	public:
		DirectionalLightComponent()
			: direction(vector3(0.0f, -1.0f, 0.0f))
			, illuminance(vector3(1.0f, 1.0f, 1.0f))
		{}

		virtual void createRenderProxy(SceneProxy* scene) override {
			DirectionalLightProxy* proxy = ALLOC_RENDER_PROXY<DirectionalLightProxy>(scene);

			proxy->wsDirection = direction;
			proxy->padding0    = 0.0f;
			proxy->illuminance = illuminance;
			proxy->padding1    = 0.0f;
			proxy->vsDirection = vector3(0.0f); // This is filled later
			proxy->padding2    = 0.0f;

			scene->proxyList_directionalLight.push_back(proxy);
		}

	public:
		vector3 direction;
		vector3 illuminance; // Unit: lux (= lm/m^2 = lumen per square meter)

	};

}
