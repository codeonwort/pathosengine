#pragma once

#include "reflection_probe_component.h"

namespace pathos {

	class ReflectionProbeActor : public Actor {

	public:
		ReflectionProbeActor() {
			probeComponent = createDefaultComponent<ReflectionProbeComponent>();
			setAsRootComponent(probeComponent);
		}

		void captureScene();

		void setCaptureRadius(float radius) { probeComponent->captureRadius = radius; }

		inline ReflectionProbeComponent* getProbeComponent() const { return probeComponent; }

		inline uint32 internal_getUpdatePhase() const { return updatePhase; }

	public:
		bool bUpdateEveryFrame = true;
		float lastUpdateTime = -1.0f;

	private:
		ReflectionProbeComponent* probeComponent;
		uint32 updatePhase = 0;

	};

}
