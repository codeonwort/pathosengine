#pragma once

#include <glm/glm.hpp>
#include <pathos/wrapper/transform.h>

namespace pathos {

	class Lens {
	public:
		virtual glm::mat4 getProjectionMatrix() = 0;
	};

	class PerspectiveLens : public Lens {
	private:
		glm::mat4 transform;
	public:
		PerspectiveLens(float fov, float aspect, float znear, float zfar);
		virtual glm::mat4 getProjectionMatrix();
	};

	class Camera {
	private:
		Transform transform; // view transform
		Lens* lens;
	public:
		Camera(Lens* lens);
		glm::mat4 getViewMatrix();
		glm::mat4 getViewProjectionMatrix();
		glm::vec3 getEyeVector();
		void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
		void move(const glm::vec3& movement);
		void rotate(float angleDegree, const glm::vec3& axis);

	};
}