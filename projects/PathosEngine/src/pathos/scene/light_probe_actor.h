#pragma once

#include "light_probe_component.h"

namespace pathos {

	// #todo-light-probe: Rename to ReflectionProbeActor.
	class LightProbeActor : public Actor {

	public:
		LightProbeActor() {
			probeComponent = createDefaultComponent<LightProbeComponent>();
			setAsRootComponent(probeComponent);
		}

		void captureScene();

		void setCaptureRadius(float radius) { probeComponent->captureRadius = radius; }

		inline LightProbeComponent* getProbeComponent() const { return probeComponent; }

		inline uint32 internal_getUpdatePhase() const { return updatePhase; }

	public:
		bool bUpdateEveryFrame = true;
		float lastUpdateTime = -1.0f;

	private:
		LightProbeComponent* probeComponent;
		uint32 updatePhase = 0;

	};

}
