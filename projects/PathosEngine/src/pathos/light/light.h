#pragma once

#include "gl_core.h"
#include <glm/glm.hpp>
#include <vector>

namespace pathos {

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
