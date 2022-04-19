#include "pathos/camera/camera.h"
#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"
#include "glm/gtc/matrix_transform.hpp"

namespace pathos {

	static const float MAX_PITCH = glm::radians(80.0f);
	static const float MIN_PITCH = glm::radians(-80.0f);

	static const glm::vec3 forward0(0.0f, 0.0f, 1.0f);
	static const glm::vec3 right0(1.0f, 0.0f, 0.0f);
	static const glm::vec3 up0(0.0f, 1.0f, 0.0f);

	// PerspectiveLens
	PerspectiveLens::PerspectiveLens(float fovY_degrees, float aspect_wh, float znear, float zfar) {
		z_near = znear;
		z_far = zfar;
		fovY_radians = glm::radians(fovY_degrees);
		aspect = aspect_wh;

		updateProjectionMatrix();
	}

	glm::mat4 PerspectiveLens::getProjectionMatrix() const {
		return transform;
	}

	void PerspectiveLens::setFovY(float inFovY_degrees) {
		fovY_radians = glm::radians(inFovY_degrees);
		updateProjectionMatrix();
	}

	void PerspectiveLens::updateProjectionMatrix() {
#if 1 // Reverse-Z
		float f = 1.0f / tan(fovY_radians / 2.0f);
		transform = glm::mat4(
			f / aspect, 0.0f, 0.0f,   0.0f,
			0.0f,       f,    0.0f,   0.0f,
			0.0f,       0.0f, 0.0f,   -1.0f,
			0.0f,       0.0f, z_near, 0.0f);
#else
		transform = glm::perspective(fovY_radians, aspect, znear, zfar);
#endif
	}

	// Camera
	Camera::Camera(const PerspectiveLens& lens)
		: lens(lens)
		, viewDirty(true)
	{
		rotationX = rotationY = 0.0f;
		_position = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	void Camera::changeLens(const PerspectiveLens& newLens) {
		lens = newLens;
	}

	void Camera::calculateViewMatrix() const {
		if (viewDirty) {
			transform.identity();
			transform.appendMove(-_position);
			transform.appendRotation(rotationY, up0);
			transform.appendRotation(rotationX, right0);
			viewDirty = false;
		}
	}
	glm::mat4 Camera::getViewMatrix() const {
		calculateViewMatrix();
		return transform.getMatrix();
	}
	glm::mat4 Camera::getViewProjectionMatrix() const {
		return lens.getProjectionMatrix() * getViewMatrix();
	}
	glm::mat4 Camera::getProjectionMatrix() const {
		return lens.getProjectionMatrix();
	}
	glm::vec3 Camera::getEyeVector() const {
		// eye direction in world space
		calculateViewMatrix();
		return transform.inverseTransformVector(-forward0);
	}
	glm::vec3 Camera::getPosition() const {
		return _position;
	}

	void Camera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
		glm::mat3 L = glm::transpose(glm::mat3(glm::lookAt(position, target, up)));
		glm::vec3 v = glm::normalize(L * forward0);
		rotationX = asin(v.y);
		if (v.z >= 0.0f)
		{
			rotationY = -asin(v.x);
		}
		else
		{
			rotationY = glm::pi<float>() + asin(v.x);
		}
		_position = position;
		viewDirty = true;
	}

	// move direction is alongside the camera's view direction
	void Camera::move(const glm::vec3& forwardRightUp) {
		calculateViewMatrix();
		_position += transform.inverseTransformVector(forwardRightUp.x * -forward0 + forwardRightUp.y * right0 + forwardRightUp.z * up0);
		viewDirty = true;
	}

	void Camera::moveForward(float amount)
	{
		calculateViewMatrix();
		_position += transform.inverseTransformVector(amount * -forward0); // #todo-camera: Dunno why the hell I have to flip the sign
		viewDirty = true;
	}

	void Camera::moveRight(float amount)
	{
		calculateViewMatrix();
		_position += transform.inverseTransformVector(amount * right0);
		viewDirty = true;
	}

	void Camera::moveUp(float amount)
	{
		calculateViewMatrix();
		_position += transform.inverseTransformVector(amount * up0);
		viewDirty = true;
	}

	void Camera::moveToPosition(const glm::vec3& newPosition)
	{
		_position = newPosition;
		viewDirty = true;
	}

	void Camera::rotateYaw(float angleDegree) {
		rotationY += glm::radians(angleDegree);
		viewDirty = true;
	}
	void Camera::rotatePitch(float angleDegree) {
		rotationX = badger::clamp(MIN_PITCH, rotationX + glm::radians(angleDegree), MAX_PITCH);
		viewDirty = true;
	}
	
	void Camera::setYaw(float newYaw) {
		rotationY = glm::radians(newYaw);
		viewDirty = true;
	}

	void Camera::setPitch(float newPitch) {
		rotationX = glm::radians(newPitch);
		viewDirty = true;
	}

	float Camera::getYaw() const { return glm::degrees(rotationY); }
	float Camera::getPitch() const { return glm::degrees(rotationX); }

	void Camera::getFrustum(std::vector<glm::vec3>& outFrustum, uint32 numCascades) const {
		CHECK(numCascades >= 1);

		const float zn = lens.getZNear();
		const float zf = lens.getZFar();
		const float hh_near = zn * tanf(lens.getFovYRadians() * 0.5f);
		const float hw_near = hh_near * lens.getAspectRatioWH();
		const float hh_far = zf * tanf(lens.getFovYRadians() * 0.5f);
		const float hw_far = hh_far * lens.getAspectRatioWH();

		const glm::vec3 P0 = getPosition();
		const glm::mat3 viewInv = glm::transpose(glm::mat3(getViewMatrix()));

		float zi, hwi, hhi;
		outFrustum.resize(4 * (1 + numCascades));
		for (uint32 i = 0u; i <= numCascades; ++i) {
			float k = static_cast<float>(i) / static_cast<float>(numCascades);
			// #todo-shadow: Needs exponential division. Close view needs far more precision than just 1/n range of depths.
			//k = powf(k, 2.0f);

			zi = zn + (zf - zn) * k;

			hwi = hw_near + (hw_far - hw_near) * k;
			hhi = hh_near + (hh_far - hh_near) * k;

			// #todo-shadow: Dunno why the hell I have to flip the sign
			// And... debug visualization is wrong but the rendering is alright???
			outFrustum[i * 4 + 0] = (-forward0 * zi) + (right0 * hwi) + (up0 * hhi);
			outFrustum[i * 4 + 1] = (-forward0 * zi) - (right0 * hwi) + (up0 * hhi);
			outFrustum[i * 4 + 2] = (-forward0 * zi) + (right0 * hwi) - (up0 * hhi);
			outFrustum[i * 4 + 3] = (-forward0 * zi) - (right0 * hwi) - (up0 * hhi);

			outFrustum[i * 4 + 0] = P0 + (viewInv * outFrustum[i * 4 + 0]);
			outFrustum[i * 4 + 1] = P0 + (viewInv * outFrustum[i * 4 + 1]);
			outFrustum[i * 4 + 2] = P0 + (viewInv * outFrustum[i * 4 + 2]);
			outFrustum[i * 4 + 3] = P0 + (viewInv * outFrustum[i * 4 + 3]);
		}
	}

}
