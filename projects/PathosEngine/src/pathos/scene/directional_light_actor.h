#pragma once

#include "pathos/scene/actor.h"
#include "directional_light_component.h"

namespace pathos {

	class DirectionalLightActor : public Actor {

	public:
		DirectionalLightActor() {
			lightComponent = createDefaultComponent<DirectionalLightComponent>();

			setAsRootComponent(lightComponent);
		}

		void setDirection(const vector3& direction)     { lightComponent->direction = glm::normalize(direction); }
		void setIlluminance(const vector3& illuminance) { lightComponent->illuminance = illuminance; }

	private:
		DirectionalLightComponent* lightComponent;

	};

}
