#include "pathos/camera/camera.h"
#include "glm/gtc/matrix_transform.hpp"

namespace pathos {

	// PerspectiveLens
	PerspectiveLens::PerspectiveLens(float fovY_degrees, float aspect_wh, float znear, float zfar) {
		z_near = znear;
		z_far = zfar;
		fovY_half = glm::radians(fovY_degrees);
		aspect = aspect_wh;

#if 1 // Reverse-Z
		float f = 1.0f / tan(glm::radians(fovY_degrees) / 2.0f);
		transform = glm::mat4(
			f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, z_near, 0.0f);
#else
		transform = glm::perspective(fovY_half, aspect, znear, zfar);
#endif
	}

	glm::mat4 PerspectiveLens::getProjectionMatrix() const {
		return transform;
	}

	// Camera
	Camera::Camera(Lens* lens) :lens(lens), viewDirty(true) {
		rotationX = rotationY = 0.0f;
		movement = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	void Camera::calculateViewMatrix() const {
		if (viewDirty){
			transform.identity();
			transform.appendMove(movement);
			
			transform.appendRotation(rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
			transform.appendRotation(rotationX, glm::vec3(1.0f, 0.0f, 0.0f));
			viewDirty = false;
		}
	}
	glm::mat4 Camera::getViewMatrix() const {
		calculateViewMatrix();
		return transform.getMatrix();
	}
	glm::mat4 Camera::getViewProjectionMatrix() const {
		return lens->getProjectionMatrix() * getViewMatrix();
	}
	glm::mat4 Camera::getProjectionMatrix() const {
		return lens->getProjectionMatrix();
	}
	glm::vec3 Camera::getEyeVector() const {
		// eye direction in world space
		calculateViewMatrix();
		return transform.inverseTransformVector(glm::vec3(0.0f, 0.0f, -1.0f));
	}
	glm::vec3 Camera::getPosition() const {
		calculateViewMatrix();
		return transform.getPosition();
	}

	void Camera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
		glm::mat3 L = glm::transpose(glm::mat3(glm::lookAt(position, target, up)));
		glm::vec3 v = glm::normalize(L * glm::vec3(0.0f, 0.0f, -1.0f));
		rotationX = -asin(v.y);
		rotationY = asin(v.x);
		movement = -position;
		viewDirty = true;
	}

	// move direction is alongside the camera's view direction
	void Camera::move(const glm::vec3& movement_) {
		glm::vec3 mov = transform.inverseTransformVector(movement_);
		movement -= mov;
		viewDirty = true;
	}

	void Camera::rotateY(float angleDegree) {
		rotationY += glm::radians(angleDegree);
		viewDirty = true;
	}
	void Camera::rotateX(float angleDegree) {
		rotationX += glm::radians(angleDegree);
		viewDirty = true;
	}
	
	void Camera::getFrustum(std::vector<glm::vec3>& outFrustum, uint32_t numCascades) const {
		assert(numCascades >= 1);

		const glm::vec3 forward = getEyeVector();
		glm::vec3 up(0.0f, 1.0f, 0.0f);

		float theta = glm::dot(forward, up);
		if (theta >= 0.999f || theta <= -0.999f) {
			up = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		glm::vec3 right = glm::cross(forward, up);
		up = glm::cross(right, forward);

		PerspectiveLens* plens = dynamic_cast<PerspectiveLens*>(lens);
		assert(plens);
		 
		const float zn = plens->getZNear();
		const float zf = plens->getZFar();
		const float hh_near = zn * tanf(plens->getFovY() * 0.5f);
		const float hw_near = hh_near * plens->getAspectRatioWH();
		const float hh_far = zf * tanf(plens->getFovY() * 0.5f);
		const float hw_far = hh_far * plens->getAspectRatioWH();

		const glm::vec3 P0 = getPosition();
		const glm::mat4 view = getViewMatrix();

		float zi, hwi, hhi;
		outFrustum.resize(4 * (1 + numCascades));
		for (uint32_t i = 0u; i <= numCascades; ++i) {
			float k = static_cast<float>(i) / static_cast<float>(numCascades);

			zi = zn + (zf - zn) * k;
			hwi = hw_near + (hw_far - hw_near) * k;
			hhi = hh_near + (hh_far - hh_near) * k;

			outFrustum[i * 4 + 0] = P0 + (forward * zi) + (right * hwi) + (up * hhi);
			outFrustum[i * 4 + 1] = P0 + (forward * zi) - (right * hwi) + (up * hhi);
			outFrustum[i * 4 + 2] = P0 + (forward * zi) + (right * hwi) - (up * hhi);
			outFrustum[i * 4 + 3] = P0 + (forward * zi) - (right * hwi) - (up * hhi);

			outFrustum[i * 4 + 0] = glm::vec3(view * glm::vec4(outFrustum[i * 4 + 0], 1.0f));
			outFrustum[i * 4 + 1] = glm::vec3(view * glm::vec4(outFrustum[i * 4 + 1], 1.0f));
			outFrustum[i * 4 + 2] = glm::vec3(view * glm::vec4(outFrustum[i * 4 + 2], 1.0f));
			outFrustum[i * 4 + 3] = glm::vec3(view * glm::vec4(outFrustum[i * 4 + 3], 1.0f));
		}
	}

}
