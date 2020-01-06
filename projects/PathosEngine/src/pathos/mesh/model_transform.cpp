#include "model_transform.h"

#include "glm/gtx/transform.hpp"

namespace pathos {

	ModelTransform::ModelTransform()
		: ModelTransform(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec3(1.0f))
	{
	}

	ModelTransform::ModelTransform(const glm::vec3& inLocation, const glm::vec3& inRotationAxis, float inRotationAngle_radians, const glm::vec3& inScale)
	{
		setLocation(inLocation);
		setRotation(inRotationAngle_radians, inRotationAxis);
		setScale(inScale);
	}

	void ModelTransform::identity()
	{
		setLocation(glm::vec3(0.0f));
		setRotation(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		setScale(glm::vec3(1.0f));
	}

	void ModelTransform::setLocation(const glm::vec3& inLocation)
	{
		location = inLocation;

		bDirty = true;
	}

	void ModelTransform::setLocation(float inX, float inY, float inZ)
	{
		setLocation(glm::vec3(inX, inY, inZ));
	}

	void ModelTransform::setRotation(float inAngle_radians, const glm::vec3& inAxis)
	{
		rotationAngle = inAngle_radians;
		rotationAxis = inAxis;

		bDirty = true;
	}

	void ModelTransform::setScale(const glm::vec3& inScale)
	{
		scale = inScale;

		bDirty = true;
	}

	void ModelTransform::setScale(float inScale)
	{
		setScale(glm::vec3(inScale));
	}

	const glm::mat4& ModelTransform::getMatrix() const
	{
		if (bDirty) {
			rawMatrix = glm::scale(glm::rotate(glm::translate(location), rotationAngle, rotationAxis), scale);
			bDirty = false;
		}

		return rawMatrix;
	}

}
