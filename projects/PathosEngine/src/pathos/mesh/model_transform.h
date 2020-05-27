#pragma once

#include "badger/types/vector_types.h"

namespace pathos {

	// Traditional scale-rotation-translation matrix
	struct ModelTransform {

		ModelTransform();
		ModelTransform(const vector3& inLocation, const vector3& inRotationAxis, float inRotationAngle_radians, const vector3& inScale);

		inline vector3 getLocation() const { return location; }
		inline void getRotation(float& outRotationAngle, vector3& outRotationAxis) { outRotationAngle = rotationAngle; outRotationAxis = rotationAxis; }
		inline vector3 getScale() const { return scale; }

		void identity();

		void setLocation(float inX, float inY, float inZ);
		void setLocation(const vector3& inLocation);

		void setRotation(float inAngle_radians, const vector3& inAxis);

		void setScale(const vector3& inScale);
		void setScale(float inScale);

		const glm::mat4& getMatrix() const;

	private:
		vector3 location;
		float rotationAngle;
		vector3 rotationAxis; // #todo-transform: quaternion or euler angles?
		vector3 scale;

		mutable bool bDirty;
		mutable glm::mat4 rawMatrix;

	};

}
