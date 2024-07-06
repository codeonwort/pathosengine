#include "reflection_probe_actor.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene.h"

// #wip-probe: If 0, update all at once for easy debugging.
#define PROGRESSIVE_UPDATE 1

namespace pathos {

	void ReflectionProbeActor::captureScene() {
#if !PROGRESSIVE_UPDATE
		for (int cnt = 0; cnt <= 6; ++cnt) {
#endif
			if (0 <= updatePhase && updatePhase < 6) {
				probeComponent->captureScene(updatePhase);
			} else if (updatePhase == 6) {
				probeComponent->bakeIBL();
			}

			if (updatePhase == 6) {
				lastUpdateTime = gEngine->getWorldTime();
			}
			updatePhase = (updatePhase + 1) % 7;
#if !PROGRESSIVE_UPDATE
		}
#endif
	}

	void ReflectionProbeActor::onSpawn() {
		getWorld()->getScene().registerReflectionProbe(this);
	}

	void ReflectionProbeActor::onDestroy() {
		getWorld()->getScene().unregisterReflectionProbe(this);
	}

}
