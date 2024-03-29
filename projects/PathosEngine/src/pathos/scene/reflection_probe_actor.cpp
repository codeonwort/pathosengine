#include "reflection_probe_actor.h"

namespace pathos {

	void ReflectionProbeActor::captureScene() {
		if (0 <= updatePhase && updatePhase < 6) {
			probeComponent->captureScene(updatePhase);
		} else if (updatePhase == 6) {
			probeComponent->bakeIBL();
		}

		if (updatePhase == 6) {
			lastUpdateTime = gEngine->getWorldTime();
		}
		updatePhase = (updatePhase + 1) % 7;
	}

}
