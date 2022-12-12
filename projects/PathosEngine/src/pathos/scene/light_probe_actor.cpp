#include "light_probe_actor.h"

namespace pathos {

	void LightProbeActor::captureScene() {
		probeComponent->captureScene(updatePhase);
		updatePhase = (updatePhase + 1) % 6;
	}

}
