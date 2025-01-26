#pragma once

#include "pathos/scene/scene_component.h"

namespace pathos {

	struct CascadedShaowMapSettings {
		bool bCastShadows = true;
		uint32 cascadeCount = 4;
		uint32 size = 2048; // both width and height
		float zFar = 500.0f;
	};

	// Memory layout should match with DirectionalLight in light.hlsl
	struct DirectionalLightProxy : public SceneComponentProxy {
		vector3 directionWS;
		uint32  bCastShadows;
		vector3 intensity; // color * illuminance
		uint32  shadowMapCascadeCount;
		vector3 directionVS;
		uint32  shadowMapSize;
		float   shadowMapZFar;
		vector3 _pad0;

		static DirectionalLightProxy createDummy() {
			CascadedShaowMapSettings defaultShadows{};
			DirectionalLightProxy dummy;
			dummy.directionWS           = vector3(0.0f, -1.0f, 0.0f);
			dummy.bCastShadows          = (uint32)defaultShadows.bCastShadows;
			dummy.intensity             = vector3(0.0f);
			dummy.shadowMapCascadeCount = defaultShadows.cascadeCount;
			dummy.directionVS           = vector3(0.0f, -1.0f, 0.0f);
			dummy.shadowMapSize         = defaultShadows.size;
			dummy.shadowMapZFar         = defaultShadows.zFar;
			dummy._pad0                 = vector3(0.0f);
			return dummy;
		}

		inline vector3 getIntensity() const { return intensity; }
	};

	class DirectionalLightComponent : public SceneComponent {

	public:
		DirectionalLightComponent();

		virtual void createRenderProxy(SceneProxy* scene) override;

	public:
		vector3 direction;   // From sun to earth
		vector3 color;       // Luminous efficiency function. Should be clamped to [0, 1]
		float   illuminance; // Unit: lux (= lm/m^2 = lumen per square meter)

		CascadedShaowMapSettings shadowSettings;

	};

}
