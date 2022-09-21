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

		void setAttenuationRadius(float radius) {
			lightComponent->attenuationRadius = radius;
		}

		void setOuterAngle(float outerAngle) {
			if (outerAngle < 0.1f) outerAngle = 0.1f;
			else if (outerAngle > 178.0f) outerAngle = 178.0f;
			lightComponent->outerAngle = outerAngle;
		}

	private:
		RectLightComponent* lightComponent;

	};

}
