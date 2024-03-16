#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/scene/actor.h"
#include "pathos/scene/volumetric_cloud_component.h"

namespace pathos {

	class Texture;
	class VolumetricCloudComponent;

	// Just spawn a VolumetricCloudActor then its attached component will create a scene proxy.
	// CAUTION: If multiple cloud actors exist, only one is rendered.
	class VolumetricCloudActor : public Actor {

	public:
		VolumetricCloudActor() {
			cloudComponent = createDefaultComponent<VolumetricCloudComponent>();
			setAsRootComponent(cloudComponent);
		}

		~VolumetricCloudActor() = default;

		void setTextures(Texture* inWeatherTexture, Texture* inShapeNoise, Texture* inErosionNoise) {
			cloudComponent->setTextures(inWeatherTexture, inShapeNoise, inErosionNoise);
		}

		inline VolumetricCloudComponent* getCloudComponent() const { return cloudComponent; }

	private:
		VolumetricCloudComponent* cloudComponent;

	};

}
