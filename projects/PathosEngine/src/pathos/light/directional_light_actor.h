#pragma once

#include "pathos/actor/actor.h"
#include "pathos/light/directional_light_component.h"

namespace pathos {

	class DirectionalLightActor : public Actor {

	public:
		DirectionalLightActor() {
			lightComponent = createDefaultComponent<DirectionalLightComponent>();
		}

		// #todo-light: Temporary API
		void setLightParameters(
			const glm::vec3& inDirection,
			const glm::vec3& inRadiance)
		{
			lightComponent->direction = inDirection;
			lightComponent->radiance = inRadiance;
		}

	private:
		DirectionalLightComponent* lightComponent;

	};

}
