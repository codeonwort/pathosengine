#pragma once

#include "pathos/scene/scene_component.h"

namespace pathos {

	struct CascadedShaowMapSettings {
		// Changing it greater than 4 will break the program. ex) see UBO_PerFrame.
		static constexpr uint32 MAX_CASCADE_COUNT = 4u;

		bool   bCastShadows = true;
		uint32 cascadeCount = 4;
		uint32 size         = 2048;   // Both width and height.
		float  zFar         = 500.0f; // Light frustum could be too large if we use camera's zFar as is.
	};

	// Memory layout should match with DirectionalLight in light.hlsl
	struct DirectionalLightProxy : public SceneComponentProxy {
		vector3 directionWS;
		uint32  bCastShadows;
		vector3 intensity; // color * illuminance
		uint32  shadowMapCascadeCount;
		vector3 directionVS;
		uint32  shadowMapSize;
		vector3 _pad0;
		float   shadowMapZFar;
		// For shadowmap sampling. They make the uniform buffer for DirectionalLight bloated
		// but there's rarely two or more directional lights so it's OK I guess?
		matrix4 lightViewMatrices[CascadedShaowMapSettings::MAX_CASCADE_COUNT]; // 64 * 4 bytes
		matrix4 lightViewProjMatrices[CascadedShaowMapSettings::MAX_CASCADE_COUNT]; // 64 * 4 bytes
		vector4 csmZSlices;

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
			return dummy;
		}

		inline vector3 getIntensity() const { return intensity; }
	};

	class DirectionalLightComponent : public SceneComponent {

	public:
		DirectionalLightComponent();

		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		void calculateLightFrustumBounds(size_t cascadeIx, const vector3* frustum, DirectionalLightProxy* outProxy);

	public:
		vector3 direction;   // From sun to earth
		vector3 color;       // Luminous efficiency function. Should be clamped to [0, 1]
		float   illuminance; // Unit: lux (= lm/m^2 = lumen per square meter)

		CascadedShaowMapSettings shadowSettings;

	};

}
