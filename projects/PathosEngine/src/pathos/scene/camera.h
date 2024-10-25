#pragma once

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include "badger/math/plane.h"

#include "pathos/util/transform_helper.h"

#include <vector>

namespace pathos {

	// #todo-camera: Orthogonal projection
	class PerspectiveLens {
	public:
		PerspectiveLens(float fovY_degrees, float aspectRatio, float znear, float zfar);

		matrix4 getProjectionMatrix() const;

		inline float getFovYRadians() const { return fovY_radians; }
		inline float getAspectRatioWH() const { return aspect; }
		inline float getZNear() const { return z_near; }
		inline float getZFar() const { return z_far; }

		inline bool isFlipX() const { return bFlipX; }
		inline bool isFlipY() const { return bFlipY; }

		void setFovYDegrees(float inFovY_degrees);
		void setFovYRadians(float inFovY_radians);
		void setAspectRatio(float inAspectRatio);

		inline void setProjectionFlips(bool bFlipHorizontal, bool bFlipVertical) {
			bFlipX = bFlipHorizontal;
			bFlipY = bFlipVertical;
			updateProjectionMatrix();
		}

	private:
		void updateProjectionMatrix();

		matrix4 transform;
		float fovY_radians;
		float aspect; // (width / height)
		float z_near;
		float z_far;

		bool bFlipX = false, bFlipY = false;
	};

	// Free-fly camera.
	// #todo-camera: Turn into an ActorComponent?
	class Camera {
	public:
		Camera(const PerspectiveLens& lens);

		void changeLens(const PerspectiveLens& newLens);

		matrix4 getViewMatrix() const;
		matrix4 getViewProjectionMatrix() const;
		matrix4 getProjectionMatrix() const;
		vector3 getEyeVector() const;
		vector3 getPosition() const;

		inline float getZNear() const { return lens.getZNear(); }
		inline float getZFar() const { return lens.getZFar(); }
		inline float getFovYRadians() const { return lens.getFovYRadians(); }
		inline float getAspectRatio() const { return lens.getAspectRatioWH(); }

		void lookAt(const vector3& origin, const vector3& target, const vector3& up);

		void move(const vector3& forwardRightUp);
		void moveForward(float amount);
		void moveRight(float amount);
		void moveUp(float amount);
		void moveToPosition(const vector3& newPosition);
		inline void moveToPosition(float x, float y, float z) { moveToPosition(vector3(x, y, z)); }

		void rotateYaw(float angleDegree); // mouse left/right in first person view
		void rotatePitch(float angleDegree); // mouse up/down in first person view
		void setYaw(float newYaw);
		void setPitch(float newPitch);

		float getYaw() const;
		float getPitch() const;

		// get vertices of camera frustum in world space
		void getFrustumVertices(
			std::vector<vector3>& outFrustum,
			uint32 numCascades,
			float zFarOverride = -1.0f,
			float* outZSlices = nullptr) const;

		void getFrustumPlanes(Frustum3D& outFrustum) const;

		inline PerspectiveLens& getLens() { return lens; }

	private:
		void calculateViewMatrix() const;

		mutable bool viewDirty;
		mutable Transform transform; // view transform

		PerspectiveLens lens; // projection transform
		float rotationX; // pitch (radians)
		float rotationY; // yaw (radians)
		vector3 position;
	};
}
