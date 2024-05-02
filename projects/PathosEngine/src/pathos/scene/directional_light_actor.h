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
		void setColor(const vector3& color)         { lightComponent->color = color; }
		void setIlluminance(float illuminance)      { lightComponent->illuminance = illuminance; }

		void setColorAndIlluminance(const vector3& color, float illuminance) {
			lightComponent->color = color;
			lightComponent->illuminance = illuminance;
		}

	private:
		DirectionalLightComponent* lightComponent;

	};

}
