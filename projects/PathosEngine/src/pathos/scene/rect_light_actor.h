#pragma once

#include "pathos/scene/actor.h"
#include "rect_light_component.h"

namespace pathos {

	class RectLightActor : public Actor {

	public:
		RectLightActor() {
			lightComponent = createDefaultComponent<RectLightComponent>();
			setAsRootComponent(lightComponent);
		}

		inline RectLightComponent* getLightComponent() const {
			return lightComponent;
		}

		void setLightSize(float width, float height) {
			lightComponent->width = width;
			lightComponent->height = height;
		}

		void setLightIntensity(const vector3& intensity) {
			lightComponent->intensity = intensity;
		}

		void setAttenuationRadius(float radius) {
			lightComponent->attenuationRadius = radius;
		}

	private:
		RectLightComponent* lightComponent;

	};

}
