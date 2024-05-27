#include "reflection_probe_actor.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene.h"

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

	void ReflectionProbeActor::onSpawn() {
		getWorld()->getScene().registerReflectionProbe(this);
	}

	void ReflectionProbeActor::onDestroy() {
		getWorld()->getScene().unregisterReflectionProbe(this);
	}

}
