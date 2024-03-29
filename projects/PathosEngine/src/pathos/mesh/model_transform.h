#pragma once

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include "badger/math/rotator.h"

namespace pathos {

	// Traditional rigid transform (scale-rotation-translation matrix)
	struct ModelTransform {

		ModelTransform();
		ModelTransform(const vector3& inLocation, const Rotator& inRotation, const vector3& inScale);

		inline vector3 getLocation() const { return location; }
		inline Rotator getRotation() const { return rotation; }
		inline vector3 getScale() const { return scale; }

		void identity();

		void setLocation(float inX, float inY, float inZ);
		void setLocation(const vector3& inLocation);

		void setRotation(const Rotator& inRotation);

		void setScale(const vector3& inScale);
		void setScale(float inScale);

		const matrix4& getMatrix() const;

	private:
		vector3 location; // Unit: meter
		Rotator rotation;
		vector3 scale;

		mutable bool bDirty;
		mutable matrix4 rawMatrix;

	};

}
