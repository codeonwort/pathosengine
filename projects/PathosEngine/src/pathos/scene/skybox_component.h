#pragma once

#include "pathos/scene/scene_component.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	class Material;

	struct SkyboxProxy : SceneComponentProxy {
		CubeGeometry*    cube;
		GLuint           textureID;
		float            textureLod;
		Material*        skyboxMaterial;
		bool             bUseCubemapTexture;
		bool             bLightingDirty;
	};

	class SkyboxComponent : public SceneComponent {

	public:
		~SkyboxComponent();

		void setCubemapTexture(GLuint inTextureID);
		void setCubemapLOD(float inLOD);

		void setSkyboxMaterial(Material* inMaterial);

		inline bool hasValidResources() const {
			bool bTexture = (bUseCubemapTexture && cubeGeometry != nullptr && cubemapTextureID != 0);
			bool bMaterial = (!bUseCubemapTexture && cubeGeometry != nullptr && skyboxMaterial != nullptr);
			return bTexture || bMaterial;
		}

		bool bUseCubemapTexture = true;

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		CubeGeometry* cubeGeometry = nullptr;
		GLuint cubemapTextureID = 0;
		float cubemapLod = 0.0f;

		Material* skyboxMaterial = nullptr;

		bool bLightingDirty = false;
	};

}
