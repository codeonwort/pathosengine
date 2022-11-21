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

		void setDirection(const vector3& direction) { lightComponent->direction = glm::normalize(direction); }
		void setIlluminance(const vector3& illuminance) { lightComponent->radiance = illuminance; }

		// #todo-measurement: Temporary API
		void setLightParameters(
			const vector3& inDirection,
			const vector3& inRadiance)
		{
			lightComponent->direction = inDirection;
			lightComponent->radiance = inRadiance;
		}

	private:
		DirectionalLightComponent* lightComponent;

	};

}
