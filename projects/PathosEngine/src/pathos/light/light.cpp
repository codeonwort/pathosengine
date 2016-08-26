#include <pathos/light/light.h>

namespace pathos {

	// point light
	PointLight::PointLight(const glm::vec3& position, const glm::vec3& color) {
		this->position = position;
		this->color = color;
		for (int i = 0; i < 3; i++){
			posPtr[i] = position[i];
			colorPtr[i] = color[i];
		}
	}

	GLfloat* PointLight::getColor() { return colorPtr; }
	GLfloat* PointLight::getPosition() { return posPtr; }

	// directional light
	DirectionalLight::DirectionalLight(const glm::vec3& direction, const glm::vec3& color) {
		this->direction = glm::normalize(direction);
		this->color = color;
		for (int i = 0; i < 3; i++){
			directionPtr[i] = this->direction[i];
			colorPtr[i] = color[i];
		}
	}

	GLfloat* DirectionalLight::getColor() { return colorPtr; }
	GLfloat* DirectionalLight::getDirection() { return directionPtr; }

}