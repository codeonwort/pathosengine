#include "skybox_actor.h"

namespace pathos {

	void SkyboxActor::setCubemapTexture(GLuint inTextureID, float inLod) {
		component->setCubemapTexture(inTextureID);
		component->setCubemapLOD(inLod);
		component->bUseCubemapTexture = true;
	}

	void SkyboxActor::setSkyboxMaterial(Material* material) {
		component->setSkyboxMaterial(material);
		component->bUseCubemapTexture = false;
	}

}
