#pragma once

#include "pathos/actor/actor.h"
#include "pathos/light/rect_light_component.h"

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

	private:
		RectLightComponent* lightComponent;

	};

}
