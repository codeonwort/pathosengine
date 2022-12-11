#pragma once

#include "light_probe_component.h"

namespace pathos {

	class LightProbeActor : public Actor {

	public:
		LightProbeActor() {
			probeComponent = createDefaultComponent<LightProbeComponent>();
			setAsRootComponent(probeComponent);
		}

		void setProbeType(ELightProbeType type) { probeComponent->probeType = type; }
		void setCaptureRadius(float radius) { probeComponent->captureRadius = radius; }

		inline LightProbeComponent* getProbeComponent() const { return probeComponent; }

	private:
		LightProbeComponent* probeComponent;

	};

}
