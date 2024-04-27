#pragma once

#include "pathos/scene/scene_component.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	class Texture;
	class Material;

	struct SkyboxProxy : SceneComponentProxy {
		CubeGeometry*    cube;
		Texture*         texture;
		float            textureLod;
		float            intensityMultiplier;
		Material*        skyboxMaterial;
		bool             bUseCubemapTexture;
		bool             bLightingDirty;
	};

	class SkyboxComponent : public SceneComponent {

	public:
		~SkyboxComponent();

		void setCubemapTexture(Texture* inTexture);
		void setCubemapLOD(float inLOD);

		// If Sun uses photometric unit but sky cubemap is not a proper HDR image,
		// then sky could be too dark compared to Sun.
		void setIntensityMultiplier(float inMultiplier);

		void setSkyboxMaterial(Material* inMaterial);

		bool hasValidResources() const;

		bool bUseCubemapTexture = true;

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		CubeGeometry* cubeGeometry = nullptr;
		Texture* cubemapTexture = nullptr;
		float cubemapLod = 0.0f;
		float intensityMultiplier = 1.0f;

		Material* skyboxMaterial = nullptr;

		bool bLightingDirty = false;
	};

}
