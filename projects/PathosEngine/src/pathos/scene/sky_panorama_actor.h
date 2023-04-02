#pragma once

#include "sky_actor.h"
#include "sky_panorama_component.h"
#include "pathos/rhi/gl_handles.h"

namespace pathos {

	// Use panorama (i.e., equirectangular map) texture as sky.
	class PanoramaSkyActor : public SkyActor {

	public:
		PanoramaSkyActor() {
			component = createDefaultComponent<PanoramaSkyComponent>();
			setAsRootComponent(component);
		}

		void initialize(GLuint textureID) {
			component->initialize(textureID);
		}

		PanoramaSkyComponent* getSkyComponent() const { return component; }

	private:
		PanoramaSkyComponent* component;

	};

}
