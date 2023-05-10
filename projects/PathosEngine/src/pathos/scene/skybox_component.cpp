#include "skybox_component.h"
#include "badger/math/minmax.h"

namespace pathos {

	SkyboxComponent::~SkyboxComponent() {
		if (cubeGeometry) {
			delete cubeGeometry;
			cubeGeometry = nullptr;
		}
	}

	void SkyboxComponent::setCubemapTexture(GLuint inTextureID) {
		if (cubemapTextureID != inTextureID) {
			cubemapTextureID = inTextureID;
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

	void SkyboxComponent::setSkyboxMaterial(Material* inMaterial) {
		if (skyboxMaterial != inMaterial) {
			skyboxMaterial = inMaterial;
			bLightingDirty = true;
		}
		if (cubeGeometry == nullptr) {
			cubeGeometry = new CubeGeometry(vector3(1.0f));
		}
	}

	void SkyboxComponent::createRenderProxy(SceneProxy* scene) {
		if (!hasValidResources()) {
			scene->skybox = nullptr;
			return;
		}

		const bool bMainScene = (scene->sceneProxySource == SceneProxySource::MainScene);

		SkyboxProxy* proxy = ALLOC_RENDER_PROXY<SkyboxProxy>(scene);
		proxy->cube               = cubeGeometry;
		proxy->textureID          = cubemapTextureID;
		proxy->textureLod         = cubemapLod;
		proxy->skyboxMaterial     = skyboxMaterial;
		proxy->bUseCubemapTexture = bUseCubemapTexture;
		proxy->bLightingDirty     = bLightingDirty && bMainScene;

		if (bMainScene) {
			bLightingDirty = false;
		}

		scene->skybox = proxy;
	}

}
