#pragma once

#include "glm/glm.hpp"
#include "badger/types/int_types.h"
#include "pathos/wrapper/transform.h"

#include <vector>

namespace pathos {

	// #todo-camera: Orthogonal projection
	class PerspectiveLens {
	public:
		PerspectiveLens(float fovY_degrees, float aspectRatio, float znear, float zfar);

		glm::mat4 getProjectionMatrix() const;

		inline float getFovYRadians() const { return fovY_radians; }
		inline float getAspectRatioWH() const { return aspect; }
		inline float getZNear() const { return z_near; }
		inline float getZFar() const { return z_far; }

		void setFovY(float inFovY_degrees);
		void setAspectRatio(float inAspectRatio);

	private:
		void updateProjectionMatrix();

		glm::mat4 transform;
		float fovY_radians;
		float aspect; // (width / height)
		float z_near;
		float z_far;
	};

	// flying camera
	class Camera {
	public:
		Camera(const PerspectiveLens& lens);

		void changeLens(const PerspectiveLens& newLens);

		glm::mat4 getViewMatrix() const;
		glm::mat4 getViewProjectionMatrix() const;
		glm::mat4 getProjectionMatrix() const;
		glm::vec3 getEyeVector() const;
		glm::vec3 getPosition() const;

		inline float getZNear() const { return lens.getZNear(); }
		inline float getZFar() const { return lens.getZFar(); }
		inline float getFovYRadians() const { return lens.getFovYRadians(); }
		inline float getAspectRatio() const { return lens.getAspectRatioWH(); }

		void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

		void move(const glm::vec3& forwardRightUp);
		void moveForward(float amount);
		void moveRight(float amount);
		void moveUp(float amount);
		void moveToPosition(const glm::vec3& newPosition);
		inline void moveToPosition(float x, float y, float z) { moveToPosition(glm::vec3(x, y, z)); }

		void rotateYaw(float angleDegree); // mouse left/right in first person view
		void rotatePitch(float angleDegree); // mouse up/down in first person view
		void setYaw(float newYaw);
		void setPitch(float newPitch);

		float getYaw() const;
		float getPitch() const;

		// get vertices of camera frustum in world space
		void getFrustum(std::vector<glm::vec3>& outFrustum, uint32 numCascades) const;

		inline PerspectiveLens& getLens() { return lens; }

	private:
		void calculateViewMatrix() const;

		mutable bool viewDirty;
		mutable Transform transform; // view transform

		PerspectiveLens lens; // projection transform
		// #todo-transform: Just use Rotator...
		float rotationX; // pitch
		float rotationY; // yaw
		glm::vec3 _position;
	};
}
