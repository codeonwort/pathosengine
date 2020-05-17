#pragma once

#include "pathos/actor/actor.h"
#include "pathos/light/point_light_component.h"

// #todo-actor: Move to engine

class PointLightActor : public Actor {
	
public:
	PointLightActor()
	{
		lightComponent = new PointLightComponent;

		registerComponent(lightComponent);
	}

	void setParameters(
		const glm::vec3& inPosition,
		const glm::vec3& inColor = glm::vec3(1, 1, 1),
		float inAttenuationRadius = 100.0f,
		float inFalloffExponent = 0.001f)
	{
		lightComponent->position = inPosition;
		lightComponent->color = inColor;
		lightComponent->attenuationRadius = inAttenuationRadius;
		lightComponent->falloffExponent = inFalloffExponent;
	}

private:
	PointLightComponent* lightComponent;

};
