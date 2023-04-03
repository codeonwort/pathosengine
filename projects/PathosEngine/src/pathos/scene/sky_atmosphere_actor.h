// Simulates the Earth's atmospheric scattering.
// Currently no controls, just adapt to current Sun light settings.

#pragma once

#include "pathos/scene/sky_actor.h"
#include "pathos/scene/sky_atmosphere_component.h"

namespace pathos {

	class SkyAtmosphereActor : public SkyActor {

	public:
		SkyAtmosphereActor() {
			component = createDefaultComponent<SkyAtmosphereComponent>();
			setAsRootComponent(component);
		}

		SkyAtmosphereComponent* getComponent() const { return component; }

	private:
		SkyAtmosphereComponent* component;

	};

}
