#include "skybox_actor.h"

namespace pathos {

	void SkyboxActor::setCubemapTexture(Texture* inTexture, float inLod) {
		component->setCubemapTexture(inTexture);
		component->setCubemapLOD(inLod);
		component->bUseCubemapTexture = true;
	}

	void SkyboxActor::setSkyboxMaterial(Material* material) {
		component->setSkyboxMaterial(material);
		component->bUseCubemapTexture = false;
	}

}
