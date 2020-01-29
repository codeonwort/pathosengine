#pragma once

#include "gl_core.h"
#include <glm/glm.hpp>
#include <vector>

// Role model for parameters of point light:
//     https://docs.unrealengine.com/en-US/Engine/Rendering/LightingAndShadows/LightTypes/Point/index.html

namespace pathos {

	struct PointLightProxy {
		glm::vec3 position;
		float     attenuationRadius;
		glm::vec3 intensity;
		float     falloffExponent;
	};

	class PointLight {

	public:
		PointLight(const glm::vec3& inPosition, const glm::vec3& inColor = glm::vec3(1, 1, 1), float inAttenuationRadius = 100.0f, float inFalloffExponent = 0.001f)
			: position(inPosition)
			, color(inColor)
			, attenuationRadius(inAttenuationRadius)
			, falloffExponent(inFalloffExponent)
		{
		}

		PointLightProxy getProxy() const {
			PointLightProxy proxy;
			proxy.position          = position;
			proxy.attenuationRadius = attenuationRadius;
			proxy.intensity         = color;
			proxy.falloffExponent   = falloffExponent;

			return proxy;
		};

	public:
		glm::vec3 position;
		glm::vec3 color;
		float attenuationRadius;
		float falloffExponent;

	};

	class DirectionalLight {

	public:
		DirectionalLight(const glm::vec3& inDirection, const glm::vec3& inColor = glm::vec3(1, 1, 1))
			: direction(glm::normalize(inDirection))
			, color(inColor)
		{
		}
		inline const glm::vec3& getDirection() const { return direction; }
		inline const glm::vec3& getColor()     const { return color;     }

	private:
		glm::vec3 direction;
		glm::vec3 color;

	};

}
