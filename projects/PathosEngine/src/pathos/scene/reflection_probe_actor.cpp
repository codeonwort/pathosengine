#include "reflection_probe_actor.h"
#include "pathos/scene/world.h"
#include "pathos/scene/scene.h"

// #todo-light-probe: If 0, update all at once for easy debugging.
// CAUTION: If PROGRESSIVE_UPDATE is disabled, it has a bug that
// the render command queued by bakeIBL() is executed prior to captureScene().
// Need a fix similar that was done to IrradianceVolumeActor.
#define PROGRESSIVE_UPDATE 1

namespace pathos {

	void ReflectionProbeActor::captureScene() {
		LightProbeScene& lightScene = getWorld()->getScene().getLightProbeScene();
		Texture* cubemapArray = lightScene.getReflectionProbeArrayTexture();

#if !PROGRESSIVE_UPDATE
		for (int cnt = 0; cnt <= 6; ++cnt) {
#endif
			if (0 <= updatePhase && updatePhase < 6) {
				probeComponent->captureScene(updatePhase);
			} else if (updatePhase == 6) {
				probeComponent->bakeIBL(cubemapArray);
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
		Scene& scene = getWorld()->getScene();
		scene.registerReflectionProbe(this);

		uint32 ix = scene.lightProbeScene.allocateReflectionProbe();
		probeComponent->setCubemapIndex(ix);
	}

	void ReflectionProbeActor::onDestroy() {
		Scene& scene = getWorld()->getScene();
		scene.unregisterReflectionProbe(this);

		uint32 ix = probeComponent->getCubemapIndex();
		scene.lightProbeScene.releaseReflectionProbe(ix);
	}

}
