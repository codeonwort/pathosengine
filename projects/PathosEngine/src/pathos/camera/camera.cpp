#include <pathos/camera/camera.h>
#include <glm/gtc/matrix_transform.hpp>

namespace pathos {

	// PerspectiveLens
	PerspectiveLens::PerspectiveLens(float fov, float aspect, float znear, float zfar) {
		transform = glm::perspective(fov, aspect, znear, zfar);
	}
	glm::mat4 PerspectiveLens::getProjectionMatrix() { return transform; }

	// Camera
	Camera::Camera(Lens* lens) :lens(lens) {
		lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	}

	glm::mat4 Camera::getViewMatrix() { return transform.getMatrix(); }
	glm::mat4 Camera::getViewProjectionMatrix() { return lens->getProjectionMatrix() * transform.getMatrix(); }
	glm::vec3 Camera::getEyeVector() { return transform.inverseTransformVector(glm::vec3(0, 0, -1)); } // eye direction in world space

	void Camera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
		transform.copyFrom(glm::lookAt(position, target, up));
	}

	// move direction is alongside the camera's view direction
	void Camera::move(const glm::vec3& movement) { transform.appendMove(transform.inverseTransformVector(-movement)); }
	void Camera::rotate(float angleDegree, const glm::vec3& axis) {
		glm::vec3 pos = transform.getPosition();
		pos = transform.inverseTransformPoint(pos);
		transform.appendMove(-pos);
		transform.appendRotation(-glm::radians(angleDegree), axis);
		transform.appendMove(pos);
	}

}
