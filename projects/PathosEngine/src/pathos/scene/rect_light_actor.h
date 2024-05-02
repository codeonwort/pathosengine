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

		void setColor(const vector3& color)     { lightComponent->color = color; }
		void setIntensity(float intensity)      { lightComponent->intensity = intensity; }
		void setAttenuationRadius(float radius) { lightComponent->attenuationRadius = radius; }
		void setFalloffExponent(float exponent) { lightComponent->falloffExponent = exponent; }
		void setCastsShadow(bool value)         { lightComponent->castsShadow = value; }
		void setWidth(float width)              { lightComponent->width = width; }
		void setHeight(float height)            { lightComponent->height = height; }

		void setColorAndIntensity(const vector3& color, float intensity) {
			lightComponent->color = color;
			lightComponent->intensity = intensity;
		}

		void setLightSize(float width, float height) {
			lightComponent->width = width;
			lightComponent->height = height;
		}

		inline RectLightComponent* getLightComponent() const { return lightComponent; }

	private:
		RectLightComponent* lightComponent;

	};

}
