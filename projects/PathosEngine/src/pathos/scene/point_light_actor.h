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

		void setIntensity(const vector3& intensity) { lightComponent->intensity = intensity; }
		void setAttenuationRadius(float radius)     { lightComponent->attenuationRadius = radius; }
		void setFalloffExponent(float exponent)     { lightComponent->falloffExponent = exponent; }
		void setCastsShadow(bool value)             { lightComponent->castsShadow = value; }
		void setSourceRadius(float inRadius)        { lightComponent->sourceRadius = inRadius; }

		inline PointLightComponent* getLightComponent() const { return lightComponent; }

	private:
		PointLightComponent* lightComponent;

	};

}
