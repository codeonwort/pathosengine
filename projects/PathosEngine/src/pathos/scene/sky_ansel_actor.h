#pragma once

#include "sky_actor.h"
#include "sky_ansel_component.h"
#include "pathos/rhi/gl_handles.h"

namespace pathos {

	class AnselSkyActor : public SkyActor {

	public:
		AnselSkyActor() {
			component = createDefaultComponent<AnselSkyComponent>();
			setAsRootComponent(component);
		}

		void initialize(GLuint textureID) {
			component->initialize(textureID);
		}

		AnselSkyComponent* getSkyComponent() const { return component; }

	private:
		AnselSkyComponent* component;

	};

}
