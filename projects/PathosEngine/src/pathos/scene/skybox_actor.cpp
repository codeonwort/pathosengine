#include "skybox_actor.h"

namespace pathos {

	/**
	* @param	inTextureID		See ::loadCubemapTexture() in <pathos/loader/imageloader.h>
	*/
	void SkyboxActor::initialize(GLuint inTextureID) {
		component->initialize(inTextureID);
	}

	void SkyboxActor::setLOD(float inLOD) {
		component->setLOD(inLOD);
	}

}