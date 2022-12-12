#include "light_probe_actor.h"

namespace pathos {

	void LightProbeActor::captureScene() {
		if (0 <= updatePhase && updatePhase < 6) {
			probeComponent->captureScene(updatePhase);
		} else if (updatePhase == 6) {
			probeComponent->bakeIBL();
		}

		updatePhase = (updatePhase + 1) % 7;
		lastUpdateTime = gEngine->getWorldTime();
	}

}
