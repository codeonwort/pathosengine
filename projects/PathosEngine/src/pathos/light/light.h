#pragma once

#include "gl_core.h"
#include <glm/glm.hpp>
#include <vector>

namespace pathos {

	class PointLight {

	public:
		PointLight(const glm::vec3& inPosition, const glm::vec3& inColor = glm::vec3(1, 1, 1))
			: position(inPosition)
			, color(inColor)
		{
		}
		inline const glm::vec3& getPosition() const { return position; }
		inline const glm::vec3& getColor()    const { return color;    }

	private:
		glm::vec3 position;
		glm::vec3 color;

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
