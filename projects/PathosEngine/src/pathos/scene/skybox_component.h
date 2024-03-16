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
		Material*        skyboxMaterial;
		bool             bUseCubemapTexture;
		bool             bLightingDirty;
	};

	class SkyboxComponent : public SceneComponent {

	public:
		~SkyboxComponent();

		void setCubemapTexture(Texture* inTexture);
		void setCubemapLOD(float inLOD);

		void setSkyboxMaterial(Material* inMaterial);

		bool hasValidResources() const;

		bool bUseCubemapTexture = true;

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		CubeGeometry* cubeGeometry = nullptr;
		Texture* cubemapTexture = nullptr;
		float cubemapLod = 0.0f;

		Material* skyboxMaterial = nullptr;

		bool bLightingDirty = false;
	};

}
