#include "landscape_actor.h"
#include "landscape_component.h"

namespace pathos {

	LandscapeActor::LandscapeActor() {
		landscapeComponent = createDefaultComponent<LandscapeComponent>();
		setAsRootComponent(landscapeComponent);
	}

	void LandscapeActor::initializeSectors(float inSizeX, float inSizeY, int32 inCountX, int32 inCountY) {
		landscapeComponent->initializeSectors(inSizeX, inSizeY, inCountX, inCountY);
	}

	void LandscapeActor::initializeHeightMap(ImageBlob* heightMapBlob) {
		landscapeComponent->initializeHeightMap(heightMapBlob);
	}

	float LandscapeActor::getLandscapeY(float x, float z) const {
		vector3 pos = getActorLocation();
		vector2 uv = landscapeComponent->getNormalizedUV(x - pos.x, z - pos.z);
		float height = landscapeComponent->sampleHeightmap(uv.x, uv.y);
		height *= landscapeComponent->getHeightMultiplier();
		return pos.y + height;
	}

	vector2 LandscapeActor::getLandscapeUV(float x, float z) const {
		vector3 pos = getActorLocation();
		return landscapeComponent->getNormalizedUV(x - pos.x, z - pos.z);
	}

	void LandscapeActor::onSpawn() {
		//
	}

	void LandscapeActor::onDestroy() {
		//
	}

	void LandscapeActor::onTick(float deltaSeconds) {
		//
	}

}
