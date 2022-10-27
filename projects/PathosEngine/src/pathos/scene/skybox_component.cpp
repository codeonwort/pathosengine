#include "skybox_component.h"
#include "pathos/util/math_lib.h"

namespace pathos {

	SkyboxComponent::~SkyboxComponent() {
		if (cube) {
			delete cube;
			cube = nullptr;
		}
	}

	void SkyboxComponent::initialize(GLuint inTextureID) {
		textureID = inTextureID;
		lod = 0.0f;
		cube = new CubeGeometry(vector3(1.0f));
	}

	void SkyboxComponent::setLOD(float inLOD) {
		lod = pathos::max(0.0f, inLOD);
	}

	void SkyboxComponent::createRenderProxy(SceneProxy* scene) {
		if (!hasValidResources()) {
			scene->skybox = nullptr;
			return;
		}

		SkyboxProxy* proxy = ALLOC_RENDER_PROXY<SkyboxProxy>(scene);
		proxy->cube = cube;
		proxy->textureID = textureID;
		proxy->textureLod = lod;

		scene->skybox = proxy;
	}

}
