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

	// flying camera
	class Camera {
	private:
		bool viewDirty;
		Transform transform;	// view transform
		Lens* lens;				// projection transform
		float rotationX, rotationY;
		glm::vec3 movement;
	public:
		Camera(Lens* lens);
		glm::mat4 getViewMatrix();
		glm::mat4 getViewProjectionMatrix();
		glm::vec3 getEyeVector();
		glm::vec3 getPosition();
		void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
		void move(const glm::vec3& movement);
		//void rotate(float angleDegree, const glm::vec3& axis);
		void rotateY(float angleDegree); // mouse move to left & right in first person mode
		void rotateX(float angleDegree); // mouse move to up & down in first person mode
		void calculateViewMatrix();

	};
}