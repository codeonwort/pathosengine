#include "skybox_component.h"
#include "badger/math/minmax.h"

namespace pathos {

	SkyboxComponent::~SkyboxComponent() {
		if (cube) {
			delete cube;
			cube = nullptr;
		}
	}

	void SkyboxComponent::setCubemap(GLuint inTextureID) {
		if (textureID != inTextureID) {
			textureID = inTextureID;
			bLightingDirty = true;
		}
		if (cube == nullptr) {
			cube = new CubeGeometry(vector3(1.0f));
		}
	}

	void SkyboxComponent::setLOD(float inLOD) {
		lod = badger::max(0.0f, inLOD);
		bLightingDirty = true;
	}

	void SkyboxComponent::createRenderProxy(SceneProxy* scene) {
		if (!hasValidResources()) {
			scene->skybox = nullptr;
			return;
		}

		const bool bMainScene = (scene->sceneProxySource == SceneProxySource::MainScene);

		SkyboxProxy* proxy = ALLOC_RENDER_PROXY<SkyboxProxy>(scene);
		proxy->cube = cube;
		proxy->textureID = textureID;
		proxy->textureLod = lod;
		proxy->bLightingDirty = bLightingDirty && bMainScene;

		if (bMainScene) {
			bLightingDirty = false;
		}

		scene->skybox = proxy;
	}

}
