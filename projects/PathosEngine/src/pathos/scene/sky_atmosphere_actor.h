// Simulates the Earth's atmosphere scattering

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
