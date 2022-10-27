#pragma once

#include "pathos/scene/sky_actor.h"
#include "pathos/scene/skybox_component.h"

namespace pathos {

	class SkyboxActor : public SkyActor {

	public:
		SkyboxActor() {
			component = createDefaultComponent<SkyboxComponent>();
			setAsRootComponent(component);
		}

		void initialize(GLuint textureID);
		void setLOD(float inLOD);

		inline SkyboxComponent* getSkyboxComponent() const { return component; }

	private:
		SkyboxComponent* component;
	};

}
