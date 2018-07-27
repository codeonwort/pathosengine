#pragma once

#include "glm/glm.hpp"
#include "pathos/wrapper/transform.h"

#include <vector>

namespace pathos {

	class Lens {
	public:
		virtual glm::mat4 getProjectionMatrix() const = 0;
	};

	class PerspectiveLens : public Lens {
	public:
		PerspectiveLens(float fov, float aspect, float znear, float zfar);
		virtual glm::mat4 getProjectionMatrix() const;
		inline const float getFovYHalf() const { return fovY_half; }
		inline const float getAspectRatioWH() const { return aspect; }
		inline const float getZNear() const { return z_near; }
		inline const float getZFar() const { return z_far; }
	private:
		glm::mat4 transform;
		float fovY_half;
		float aspect; // (width / height)
		float z_near;
		float z_far;
	};

	// flying camera
	class Camera {
	public:
		Camera(Lens* lens);

		glm::mat4 getViewMatrix() const;
		glm::mat4 getViewProjectionMatrix() const;
		glm::mat4 getProjectionMatrix() const;
		glm::vec3 getEyeVector() const;
		glm::vec3 getPosition() const;

		void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
		void move(const glm::vec3& movement);

		// mouse move to left/right in first person mode
		void rotateY(float angleDegree);
		// mouse move to up/down in first person mode
		void rotateX(float angleDegree);

		// get vertices of camera frustum in world space
		void getFrustum(std::vector<glm::vec3>& outFrustum) const;

	private:
		void calculateViewMatrix() const;

		mutable bool viewDirty;
		mutable Transform transform; // view transform

		Lens* lens; // projection transform
		float rotationX;
		float rotationY;
		glm::vec3 movement;
	};
}
