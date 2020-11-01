#pragma once

#include "pathos/actor/actor.h"
#include "pathos/light/point_light_component.h"

namespace pathos {

	class PointLightActor : public Actor {

	public:
		PointLightActor()
		{
			lightComponent = createDefaultComponent<PointLightComponent>();

			setAsRootComponent(lightComponent);
		}

		// #todo-light: Temporary API
		void setLightParameters(
			const vector3& inRadiance = vector3(1.0f, 1.0f, 1.0f),
			float inAttenuationRadius = 100.0f,
			float inFalloffExponent = 0.001f,
			bool castsShadow = true)
		{
			lightComponent->color = inRadiance;
			lightComponent->attenuationRadius = inAttenuationRadius;
			lightComponent->falloffExponent = inFalloffExponent;
			lightComponent->castsShadow = castsShadow;
		}

	private:
		PointLightComponent* lightComponent;

	};

}
