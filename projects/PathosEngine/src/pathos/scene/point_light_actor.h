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

		// #todo-measurement: Temporary API
		void setLightParameters(
			const vector3& inIntensity = vector3(1.0f, 1.0f, 1.0f),
			float inAttenuationRadius = 1.0f,
			float inFalloffExponent = 0.001f,
			bool castsShadow = true)
		{
			lightComponent->intensity = inIntensity;
			lightComponent->attenuationRadius = inAttenuationRadius;
			lightComponent->falloffExponent = inFalloffExponent;
			lightComponent->castsShadow = castsShadow;
		}

		inline PointLightComponent* getLightComponent() const { return lightComponent; }

	private:
		PointLightComponent* lightComponent;

	};

}
