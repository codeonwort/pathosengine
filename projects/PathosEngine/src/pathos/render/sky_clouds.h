#pragma once

#include "pathos/actor/actor.h"
#include "gl_core.h"

namespace pathos {

	class VolumeTexture;

	// NOTE: Should be assigned to the Scene::cloud member variable.
	class VolumetricCloudActor : public Actor {

	public:
		VolumetricCloudActor() = default;
		virtual ~VolumetricCloudActor() = default;

		void setNoiseTextures(VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise);

	private:
		VolumeTexture* shapeNoise = nullptr;
		VolumeTexture* erosionNoise = nullptr;

	};

}
