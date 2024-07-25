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
