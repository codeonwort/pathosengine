#include "landscape_actor.h"
#include "landscape_component.h"

namespace pathos {

	LandscapeActor::LandscapeActor() {
		landscapeComponent = createDefaultComponent<LandscapeComponent>();
		setAsRootComponent(landscapeComponent);

		// The landscape geometry is XY plane, its normal pointing +Z.
		// Pitch -90 rotates the bottom-left vertex from (0, 0, 0) to (0, 0, 0)
		// and the top-right vertex from (w, h, 0) to (w, 0, -h).
		// So... calculating sector position, orientation, and uv all messed up :/
		// At least I made them work with a little awkward transformation.
		setActorRotation(Rotator(0.0f, -90.0f, 0.0f));
	}

	void LandscapeActor::initializeSectors(float inSizeX, float inSizeY, int32 inCountX, int32 inCountY) {
		landscapeComponent->initializeSectors(inSizeX, inSizeY, inCountX, inCountY);
	}

	void LandscapeActor::initializeHeightMap(ImageBlob* heightMapBlob) {
		landscapeComponent->initializeHeightMap(heightMapBlob);
	}

	float LandscapeActor::getLandscapeY(float x, float z) const {
		vector3 pos = getActorLocation();
		vector2 uv = landscapeComponent->getNormalizedUV(x - pos.x, pos.z - z);
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
