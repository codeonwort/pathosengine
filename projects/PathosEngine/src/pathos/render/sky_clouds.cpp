#include "sky_clouds.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/thread/engine_thread.h"
#include "pathos/texture/volume_texture.h"

// VolumetricCloudComponent
namespace pathos {

	void VolumetricCloudComponent::setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise) {
		weatherTexture = inWeatherTexture;
		shapeNoise = inShapeNoise;
		erosionNoise = inErosionNoise;
	}

	bool VolumetricCloudComponent::hasValidResources() const {
		return weatherTexture != 0
			&& shapeNoise != nullptr && shapeNoise->isValid()
			&& erosionNoise != nullptr && erosionNoise->isValid();
	}

}

// VolumetricCloudActor
namespace pathos {

	VolumetricCloudActor::VolumetricCloudActor() {
		cloudComponent = createDefaultComponent<VolumetricCloudComponent>();
		setAsRootComponent(cloudComponent);
	}

	void VolumetricCloudActor::setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise) {
		cloudComponent->setTextures(inWeatherTexture, inShapeNoise, inErosionNoise);
	}

}
