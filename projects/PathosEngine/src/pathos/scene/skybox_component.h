#pragma once

#include "pathos/scene/scene_component.h"
#include "pathos/scene/sky_common.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/smart_pointer.h"

namespace pathos {

	class Texture;
	class Material;
	class MaterialProxy;

	struct SkyboxProxy : SceneComponentProxy {
		CubeGeometry*           cube;
		Texture*                texture;
		float                   textureLod;
		float                   intensityMultiplier;
		MaterialProxy*          skyboxMaterial;
		bool                    bUseCubemapTexture;
		bool                    bLightingDirty;
		ESkyLightingUpdateMode  lightingMode;
		ESkyLightingUpdatePhase lightingPhase;
	};

	class SkyboxComponent : public SceneComponent {

	public:
		~SkyboxComponent();

		void setCubemapTexture(Texture* inTexture);
		void setCubemapLOD(float inLOD);

		// If Sun uses photometric unit but sky cubemap is not a proper HDR image,
		// then sky could be too dark compared to Sun.
		void setIntensityMultiplier(float inMultiplier);

		void setSkyboxMaterial(assetPtr<Material> inMaterial);

		bool hasValidResources() const;

		bool bUseCubemapTexture = true;

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		CubeGeometry* cubeGeometry = nullptr;
		Texture* cubemapTexture = nullptr;
		float cubemapLod = 0.0f;
		float intensityMultiplier = 1.0f;

		assetPtr<Material> skyboxMaterial;

		ESkyLightingUpdatePhase lightingUpdatePhase = (ESkyLightingUpdatePhase)0;
	};

}
