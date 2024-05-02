#pragma once

#include "pathos/scene/actor.h"
#include "point_light_component.h"

namespace pathos {

	class PointLightActor : public Actor {

	public:
		PointLightActor()
		{
			lightComponent = createDefaultComponent<PointLightComponent>();

			setAsRootComponent(lightComponent);
		}

		void setColor(const vector3& color)     { lightComponent->color = color; }
		void setIntensity(float intensity)      { lightComponent->intensity = intensity; }
		void setAttenuationRadius(float radius) { lightComponent->attenuationRadius = radius; }
		void setFalloffExponent(float exponent) { lightComponent->falloffExponent = exponent; }
		void setCastsShadow(bool value)         { lightComponent->castsShadow = value; }
		void setSourceRadius(float inRadius)    { lightComponent->sourceRadius = inRadius; }

		void setColorAndIntensity(const vector3& color, float intensity) {
			lightComponent->color = color;
			lightComponent->intensity = intensity;
		}

		inline PointLightComponent* getLightComponent() const { return lightComponent; }

	private:
		PointLightComponent* lightComponent;

	};

}
