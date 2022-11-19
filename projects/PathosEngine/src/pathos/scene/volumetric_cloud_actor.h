#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/scene/actor.h"
#include "pathos/scene/volumetric_cloud_component.h"

namespace pathos {

	class VolumeTexture;
	class VolumetricCloudComponent;

	// Placeholder for world logic.
	// NOTE: Should be assigned to the Scene::cloud member variable.
	class VolumetricCloudActor : public Actor {

	public:
		VolumetricCloudActor() {
			cloudComponent = createDefaultComponent<VolumetricCloudComponent>();
			setAsRootComponent(cloudComponent);
		}

		~VolumetricCloudActor() = default;

		void setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise) {
			cloudComponent->setTextures(inWeatherTexture, inShapeNoise, inErosionNoise);
		}

		inline VolumetricCloudComponent* getCloudComponent() const { return cloudComponent; }

	private:
		VolumetricCloudComponent* cloudComponent;

	};

}
