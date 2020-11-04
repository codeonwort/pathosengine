#include "sky_clouds.h"

namespace pathos {

	void VolumetricCloudActor::setNoiseTextures(VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise)
	{
		shapeNoise = inShapeNoise;
		erosionNoise = inErosionNoise;
	}

}
