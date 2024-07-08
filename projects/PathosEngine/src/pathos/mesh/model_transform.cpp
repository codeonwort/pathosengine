#include "model_transform.h"

#include "glm/gtx/transform.hpp"

namespace pathos {

	ModelTransform::ModelTransform()
		: ModelTransform(vector3(0.0f), Rotator(), vector3(1.0f))
	{
	}

	ModelTransform::ModelTransform(const vector3& inLocation, const Rotator& inRotation, const vector3& inScale)
	{
		setLocation(inLocation);
		setRotation(inRotation);
		setScale(inScale);
	}

	void ModelTransform::identity()
	{
		setLocation(vector3(0.0f));
		setRotation(Rotator());
		setScale(vector3(1.0f));
	}

	void ModelTransform::setLocation(const vector3& inLocation)
	{
		location = inLocation;

		bDirty = true;
	}

	void ModelTransform::setLocation(float inX, float inY, float inZ)
	{
		setLocation(vector3(inX, inY, inZ));
	}

	void ModelTransform::setRotation(const Rotator& inRotation)
	{
		rotation = inRotation;

		bDirty = true;
	}

	void ModelTransform::setScale(const vector3& inScale)
	{
		scale = inScale;

		bDirty = true;
	}

	void ModelTransform::setScale(float inScale)
	{
		setScale(vector3(inScale));
	}

	const matrix4& ModelTransform::getMatrix() const
	{
		if (bDirty) {
			rawMatrix = glm::translate(location) * rotation.toMatrix() * glm::scale(scale);
			bDirty = false;
		}

		return rawMatrix;
	}

}
