#include <pathos/camera/camera.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
using namespace std;

namespace pathos {

	// PerspectiveLens
	PerspectiveLens::PerspectiveLens(float fov, float aspect, float znear, float zfar) {
		transform = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}
	glm::mat4 PerspectiveLens::getProjectionMatrix() { return transform; }

	// Camera
	Camera::Camera(Lens* lens) :lens(lens), viewDirty(true) {
		rotationX = rotationY = 0;
		movement = glm::vec3(0, 0, 0);
	}

	void Camera::calculateViewMatrix() {
		if (viewDirty){
			transform.identity();
			transform.appendRotation(rotationX, glm::vec3(1, 0, 0));
			transform.appendRotation(rotationY, glm::vec3(0, 1, 0));
			transform.appendMove(movement);
			viewDirty = false;
		}
	}
	glm::mat4 Camera::getViewMatrix() { calculateViewMatrix(); return transform.getMatrix(); }
	glm::mat4 Camera::getViewProjectionMatrix() { return lens->getProjectionMatrix() * getViewMatrix(); }
	glm::vec3 Camera::getEyeVector() { calculateViewMatrix(); return transform.inverseTransformVector(glm::vec3(0, 0, -1)); } // eye direction in world space
	glm::vec3 Camera::getPosition() { calculateViewMatrix(); return transform.getPosition(); }

	void Camera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
		glm::mat3 L = glm::transpose(glm::mat3(glm::lookAt(position, target, up)));
		glm::vec3 v = glm::normalize(L * glm::vec3(0, 0, -1));
		rotationX = -asin(v.y);
		rotationY = asin(v.x);
		movement = -position;
		viewDirty = true;
	}

	// move direction is alongside the camera's view direction
	void Camera::move(const glm::vec3& movement) {
		glm::vec3 mov = transform.inverseTransformVector(movement);
		this->movement -= mov;
		viewDirty = true;
	}
	/*void Camera::rotate(float angleDegree, const glm::vec3& axis) {
		glm::vec3 pos = transform.getPosition();
		pos = transform.inverseTransformPoint(pos);
		transform.appendMove(-pos);
		transform.appendRotation(-glm::radians(angleDegree), axis);
		transform.appendMove(pos);
	}*/

	void Camera::rotateY(float angleDegree) {
		rotationY += glm::radians(angleDegree);
		viewDirty = true;
	}
	void Camera::rotateX(float angleDegree) {
		rotationX += glm::radians(angleDegree);
		viewDirty = true;
	}

}
