#pragma once

#include "pathos/scene/scene_component.h"

namespace pathos {

	struct DirectionalLightProxy : public SceneComponentProxy {
		vector3 wsDirection;
		float   padding0;
		vector3 intensity; // #wip: Separate into color and illuminance?
		float   padding1;
		vector3 vsDirection;
		float   padding2;

		static DirectionalLightProxy createDummy() {
			DirectionalLightProxy dummy;
			dummy.wsDirection = vector3(0.0f, -1.0f, 0.0f);
			dummy.intensity   = vector3(0.0f);
			dummy.vsDirection = vector3(0.0f, -1.0f, 0.0f);
			return dummy;
		}

		inline vector3 getIntensity() const { return intensity; }
	};

	class DirectionalLightComponent : public SceneComponent {

	public:
		DirectionalLightComponent()
			: direction(vector3(0.0f, -1.0f, 0.0f))
			, color(vector3(1.0f, 1.0f, 1.0f))
			, illuminance(1.0f)
		{}

		virtual void createRenderProxy(SceneProxy* scene) override {
			DirectionalLightProxy* proxy = ALLOC_RENDER_PROXY<DirectionalLightProxy>(scene);

			proxy->wsDirection = direction;
			proxy->padding0    = 0.0f;
			proxy->intensity   = color * illuminance;
			proxy->padding1    = 0.0f;
			proxy->vsDirection = vector3(0.0f); // This is filled later
			proxy->padding2    = 0.0f;

			scene->proxyList_directionalLight.push_back(proxy);
		}

	public:
		vector3 direction; // From sun to earth

		// #wip: Sun uses color (vector) + illuminance (scalar). If gonna support color temperature, provide a mechanism convert it to color.
		vector3 color;     // Should be clamped to [0, 1]
		float illuminance; // Unit: lux (= lm/m^2 = lumen per square meter)

	};

}
