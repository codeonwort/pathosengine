#include "pathos/camera/camera.h"
#include "pathos/engine_policy.h"

#include "badger/math/minmax.h"
#include "badger/assertion/assertion.h"
#include "glm/gtc/matrix_transform.hpp"

namespace pathos {

	static const float MAX_PITCH = glm::radians(80.0f);
	static const float MIN_PITCH = glm::radians(-80.0f);

	static const vector3 forward0(0.0f, 0.0f, 1.0f);
	static const vector3 right0(1.0f, 0.0f, 0.0f);
	static const vector3 up0(0.0f, 1.0f, 0.0f);

	// PerspectiveLens
	PerspectiveLens::PerspectiveLens(float fovY_degrees, float aspect_wh, float znear, float zfar) {
		z_near = znear;
		z_far = zfar;
		fovY_radians = glm::radians(fovY_degrees);
		aspect = aspect_wh;

		updateProjectionMatrix();
	}

	matrix4 PerspectiveLens::getProjectionMatrix() const {
		return transform;
	}

	void PerspectiveLens::setFovY(float inFovY_degrees) {
		fovY_radians = glm::radians(inFovY_degrees);
		updateProjectionMatrix();
	}

	void PerspectiveLens::setAspectRatio(float inAspectRatio) {
		aspect = inAspectRatio;
		updateProjectionMatrix();
	}

	void PerspectiveLens::updateProjectionMatrix() {
		if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
			float f = 1.0f / tan(fovY_radians / 2.0f);
			// NOTE: glm matrix is column-major!
			transform = matrix4(
				f / aspect, 0.0f, 0.0f,   0.0f,
				0.0f,       f,    0.0f,   0.0f,
				0.0f,       0.0f, 0.0f,   -1.0f,
				0.0f,       0.0f, z_near, 0.0f);
		} else {
			transform = glm::perspective(fovY_radians, aspect, z_near, z_far);
		}
	}

	// Camera
	Camera::Camera(const PerspectiveLens& lens)
		: lens(lens)
		, viewDirty(true)
	{
		rotationX = rotationY = 0.0f;
		position = vector3(0.0f, 0.0f, 0.0f);
	}

	void Camera::changeLens(const PerspectiveLens& newLens) {
		lens = newLens;
	}

	void Camera::calculateViewMatrix() const {
		if (viewDirty) {
			transform.identity();
			transform.appendMove(-position);
			transform.appendRotation(rotationY, up0);
			transform.appendRotation(rotationX, right0);
			viewDirty = false;
		}
	}
	matrix4 Camera::getViewMatrix() const {
		calculateViewMatrix();
		return transform.getMatrix();
	}
	matrix4 Camera::getViewProjectionMatrix() const {
		return lens.getProjectionMatrix() * getViewMatrix();
	}
	matrix4 Camera::getProjectionMatrix() const {
		return lens.getProjectionMatrix();
	}
	vector3 Camera::getEyeVector() const {
		// eye direction in world space
		calculateViewMatrix();
		return transform.inverseTransformVector(-forward0);
	}
	vector3 Camera::getPosition() const {
		return position;
	}

	void Camera::lookAt(const vector3& origin, const vector3& target, const vector3& up) {
		matrix3 L = glm::transpose(matrix3(glm::lookAt(origin, target, up)));
		vector3 v = glm::normalize(L * forward0);
		rotationX = asin(v.y);
		if (v.z >= 0.0f)
		{
			rotationY = -asin(v.x);
		}
		else
		{
			rotationY = glm::pi<float>() + asin(v.x);
		}
		position = origin;
		viewDirty = true;
	}

	// move direction is alongside the camera's view direction
	void Camera::move(const vector3& forwardRightUp) {
		calculateViewMatrix();
		position += transform.inverseTransformVector(
			(forwardRightUp.x * -forward0)
			+ (forwardRightUp.y * right0)
			+ (forwardRightUp.z * up0));
		viewDirty = true;
	}

	void Camera::moveForward(float amount)
	{
		calculateViewMatrix();
		// #todo-camera: Dunno why the hell I have to flip the sign
		position += transform.inverseTransformVector(amount * -forward0);
		viewDirty = true;
	}

	void Camera::moveRight(float amount)
	{
		calculateViewMatrix();
		position += transform.inverseTransformVector(amount * right0);
		viewDirty = true;
	}

	void Camera::moveUp(float amount)
	{
		calculateViewMatrix();
		position += transform.inverseTransformVector(amount * up0);
		viewDirty = true;
	}

	void Camera::moveToPosition(const vector3& newPosition)
	{
		position = newPosition;
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

	void Camera::getFrustum(std::vector<vector3>& outFrustum, uint32 numCascades) const {
		CHECK(numCascades >= 1);

		const float zn = lens.getZNear();
		const float zf = lens.getZFar();
		const float hh_near = zn * tanf(lens.getFovYRadians() * 0.5f);
		const float hw_near = hh_near * lens.getAspectRatioWH();
		const float hh_far = zf * tanf(lens.getFovYRadians() * 0.5f);
		const float hw_far = hh_far * lens.getAspectRatioWH();

		const vector3 P0 = getPosition();
		const matrix3 viewInv = glm::transpose(matrix3(getViewMatrix()));

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
