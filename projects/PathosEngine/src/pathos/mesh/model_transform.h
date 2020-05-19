#pragma once

#include "glm/glm.hpp"

namespace pathos {

	// Traditional scale-rotation-translation matrix
	struct ModelTransform {

		ModelTransform();
		ModelTransform(const glm::vec3& inLocation, const glm::vec3& inRotationAxis, float inRotationAngle_radians, const glm::vec3& inScale);

		inline glm::vec3 getLocation() const { return location; }
		// #todo-transform: getRotation()
		inline glm::vec3 getScale() const { return scale; }

		void identity();

		void setLocation(float inX, float inY, float inZ);
		void setLocation(const glm::vec3& inLocation);

		void setRotation(float inAngle_radians, const glm::vec3& inAxis);

		void setScale(const glm::vec3& inScale);
		void setScale(float inScale);

		const glm::mat4& getMatrix() const;

	private:
		glm::vec3 location;
		float rotationAngle;
		glm::vec3 rotationAxis; // #todo-transform: quaternion or euler angles?
		glm::vec3 scale;

		mutable bool bDirty;
		mutable glm::mat4 rawMatrix;

	};

}
