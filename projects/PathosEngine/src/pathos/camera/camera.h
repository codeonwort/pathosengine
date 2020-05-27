#pragma once

#include "glm/glm.hpp"
#include "badger/types/int_types.h"
#include "pathos/wrapper/transform.h"

#include <vector>

namespace pathos {

	class Lens {
	public:
		virtual glm::mat4 getProjectionMatrix() const = 0;
	};

	class PerspectiveLens : public Lens {
	public:
		PerspectiveLens(float fovY_degrees, float aspectRatio, float znear, float zfar);
		virtual glm::mat4 getProjectionMatrix() const;
		inline float getFovY() const { return fovY_half; }
		inline float getAspectRatioWH() const { return aspect; }
		inline float getZNear() const { return z_near; }
		inline float getZFar() const { return z_far; }
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

		inline float getZNear() const { return static_cast<PerspectiveLens*>(lens)->getZNear(); }
		inline float getZFar() const { return static_cast<PerspectiveLens*>(lens)->getZFar(); }
		inline float getFovYRadians() const { return static_cast<PerspectiveLens*>(lens)->getFovY(); }
		inline float getAspectRatio() const { return static_cast<PerspectiveLens*>(lens)->getAspectRatioWH(); }

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

		inline Lens* getLens() const { return lens; }

	private:
		void calculateViewMatrix() const;

		mutable bool viewDirty;
		mutable Transform transform; // view transform

		Lens* lens; // projection transform
		// #todo-transform: Just use Rotator...
		float rotationX; // pitch
		float rotationY; // yaw
		glm::vec3 _position;
	};
}
