#pragma once

#include "pathos/actor/actor.h"
#include "directional_light_component.h"

namespace pathos {

	class DirectionalLightActor : public Actor {

	public:
		DirectionalLightActor() {
			lightComponent = createDefaultComponent<DirectionalLightComponent>();

			setAsRootComponent(lightComponent);
		}

		// #todo-light: Temporary API
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
