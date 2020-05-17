#pragma once

#include "gl_core.h"
#include <glm/glm.hpp>
#include <vector>

#define OLD_POINT_LIGHT 0 // #todo-old-point-light

// Role model for parameters of point light:
//     https://docs.unrealengine.com/en-US/Engine/Rendering/LightingAndShadows/LightTypes/Point/index.html

namespace pathos {

#if OLD_POINT_LIGHT
	struct PointLightProxy {
		glm::vec3 position;
		float     attenuationRadius;
		glm::vec3 intensity;
		float     falloffExponent;
	};
#endif

	class PointLight {

	public:
		PointLight(const glm::vec3& inPosition, const glm::vec3& inColor = glm::vec3(1, 1, 1), float inAttenuationRadius = 100.0f, float inFalloffExponent = 0.001f)
			: position(inPosition)
			, color(inColor)
			, attenuationRadius(inAttenuationRadius)
			, falloffExponent(inFalloffExponent)
		{
		}

#if OLD_POINT_LIGHT
		PointLightProxy getProxy() const {
			PointLightProxy proxy;
			proxy.position          = position;
			proxy.attenuationRadius = attenuationRadius;
			proxy.intensity         = color;
			proxy.falloffExponent   = falloffExponent;

			return proxy;
		};
#endif

	public:
		glm::vec3 position;
		glm::vec3 color;
		float attenuationRadius;
		float falloffExponent;

	};

	struct DirectionalLightProxy {
		glm::vec3 direction;
		float     padding0;
		glm::vec3 intensity;
		float     padding1;
	};

	class DirectionalLight {

	public:
		DirectionalLight(const glm::vec3& inDirection, const glm::vec3& inColor = glm::vec3(1, 1, 1))
			: direction(glm::normalize(inDirection))
			, color(inColor)
		{
		}

		DirectionalLightProxy getProxy() const {
			DirectionalLightProxy proxy;
			proxy.direction = direction;
			proxy.padding0  = 0.0f;
			proxy.intensity = color;
			proxy.padding1  = 0.0f;

			return proxy;
		}

	public:
		glm::vec3 direction;
		glm::vec3 color;

	};

}
