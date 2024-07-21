#include "skybox_component.h"
#include "pathos/rhi/texture.h"

#include "badger/math/minmax.h"

namespace pathos {

	SkyboxComponent::~SkyboxComponent() {
		if (cubeGeometry) {
			delete cubeGeometry;
			cubeGeometry = nullptr;
		}
	}

	void SkyboxComponent::setCubemapTexture(Texture* inTexture) {
		if (cubemapTexture != inTexture) {
			cubemapTexture = inTexture;
			bLightingDirty = true;
		}
		if (cubeGeometry == nullptr) {
			cubeGeometry = new CubeGeometry(vector3(1.0f));
		}
	}

	void SkyboxComponent::setCubemapLOD(float inLOD) {
		cubemapLod = badger::max(0.0f, inLOD);
		bLightingDirty = true;
	}

	void SkyboxComponent::setIntensityMultiplier(float inMultiplier) {
		intensityMultiplier = inMultiplier;
		bLightingDirty = true;
	}

	void SkyboxComponent::setSkyboxMaterial(Material* inMaterial) {
		if (skyboxMaterial != inMaterial) {
			skyboxMaterial = inMaterial;
			bLightingDirty = true;
		}
		if (cubeGeometry == nullptr) {
			cubeGeometry = new CubeGeometry(vector3(1.0f));
		}
	}

	bool SkyboxComponent::hasValidResources() const {
		bool bTexture = (bUseCubemapTexture && cubeGeometry != nullptr && cubemapTexture != nullptr && cubemapTexture->isCreated());
		bool bMaterial = (!bUseCubemapTexture && cubeGeometry != nullptr && skyboxMaterial != nullptr);
		return bTexture || bMaterial;
	}

	void SkyboxComponent::createRenderProxy(SceneProxy* scene) {
		if (!hasValidResources() || !getVisibility()) {
			scene->skybox = nullptr;
			return;
		}

		const bool bMainScene = (scene->sceneProxySource == SceneProxySource::MainScene);

		SkyboxProxy* proxy = ALLOC_RENDER_PROXY<SkyboxProxy>(scene);
		proxy->cube                = cubeGeometry;
		proxy->texture             = cubemapTexture;
		proxy->textureLod          = cubemapLod;
		proxy->intensityMultiplier = intensityMultiplier;
		proxy->skyboxMaterial      = skyboxMaterial;
		proxy->bUseCubemapTexture  = bUseCubemapTexture;
		proxy->bLightingDirty      = bLightingDirty && bMainScene;

		if (bMainScene) {
			bLightingDirty = false;
		}

		scene->skybox = proxy;
	}

}
