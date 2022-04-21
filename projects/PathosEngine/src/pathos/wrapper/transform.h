#pragma once

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

namespace pathos {

	// #todo-camera: Actually I need this. Don't remove but requires renaming.
	class Transform {

	protected:
		matrix4 matrix;

	public:
		Transform();
		Transform(const matrix4& matrix);

		const matrix4& getMatrix() const;
		inline void copyFrom(const matrix4& m) { matrix = m; }
		inline vector3 getPosition() const { return vector3(matrix[3]); }

		void identity();

		void append(const matrix4& t);
		void appendMove(const vector3& movement);
		void appendMove(float dx, float dy, float dz);
		void appendRotation(float angle, const vector3& axis);
		void appendScale(const vector3& scale);
		void appendScale(float sx, float sy, float sz);
		inline void appendScale(float s) { appendScale(s, s, s); }

		void prepend(const matrix4& t);
		void prependMove(const vector3& movement);
		void prependMove(float dx, float dy, float dz);
		void prependRotation(float angle, const vector3& axis);
		void prependScale(const vector3& scale);
		void prependScale(float sx, float sy, float sz);

		vector3 transformVector(const vector3& v) const;
		vector3 inverseTransformVector(const vector3& v) const;

		vector3 transformPoint(const vector3& p) const;
		vector3 inverseTransformPoint(const vector3& p) const;
	};

}
