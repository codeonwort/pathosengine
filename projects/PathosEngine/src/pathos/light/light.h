#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

namespace pathos {

	class PointLight;
	class DirectionalLight;

	class PointLight {
	private:
		 glm::vec3 position, color;
		 GLfloat posPtr[3];
		 GLfloat colorPtr[3];
	public:
		PointLight(const glm::vec3& position, const glm::vec3& color = glm::vec3(1, 1, 1));
		inline const glm::vec3& getPositionVector() { return position; }
		GLfloat* getPosition();
		GLfloat* getColor();
	};

	// affect the diffuse color of a mesh
	class DirectionalLight {
	private:
		glm::vec3 direction, color;
		GLfloat directionPtr[3];
		GLfloat colorPtr[3];
	public:
		DirectionalLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1, 1, 1));
		GLfloat* getDirection();
		GLfloat* getColor();
	};

}