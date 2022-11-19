#include "transform_helper.h"

namespace pathos {

	Transform::Transform() : matrix(matrix4(1.0f)) {}
	Transform::Transform(const matrix4& inMatrix) : matrix(inMatrix) {}

	void Transform::identity() { matrix = matrix4(1.0f); }

	void Transform::prepend(const matrix4& t) {
		matrix = t * matrix;
	}

	void Transform::prependMove(const vector3& movement) {
		matrix = glm::translate(matrix4(1.0f), movement) * matrix;
	}
	void Transform::prependMove(float dx, float dy, float dz) {
		prependMove(vector3(dx, dy, dz));
	}

	void Transform::prependRotation(float angle, const vector3& axis) {
		matrix = glm::rotate(matrix4(1.0f), angle, axis) * matrix;
	}

	void Transform::prependScale(const vector3& scale) {
		matrix = glm::scale(matrix4(1.0f), scale) * matrix;
	}
	//void Transform::prependScale(const vector3& scale) { matrix = glm::scale(matrix, scale); }
	void Transform::prependScale(float sx, float sy, float sz) {
		prependScale(vector3(sx, sy, sz));
	}

	void Transform::append(const matrix4& t) {
		matrix = matrix * t;
	}

	void Transform::appendMove(const vector3& movement) {
		matrix = glm::translate(matrix, movement);
	}

	void Transform::appendMove(float dx, float dy, float dz) {
		appendMove(vector3(dx, dy, dz));
	}

	void Transform::appendRotation(float angle, const vector3& axis) {
		matrix = glm::rotate(matrix, angle, axis);
	}

	void Transform::appendScale(const vector3& scale) {
		matrix = glm::scale(matrix, scale);
	}

	void Transform::appendScale(float sx, float sy, float sz) {
		appendScale(vector3(sx, sy, sz));
	}

	vector3 Transform::transformVector(const vector3& v) const {
		return vector3(matrix * vector4(v, 0.0f));
	}
	vector3 Transform::inverseTransformVector(const vector3& v) const {
		return vector3(glm::transpose(matrix) * vector4(v, 0.0f));
	}

	vector3 Transform::transformPoint(const vector3& v) const {
		return vector3(matrix * vector4(v, 1.0f));
	}
	vector3 Transform::inverseTransformPoint(const vector3& v) const {
		return vector3(glm::transpose(matrix) * vector4(v, 1.0f));
	}

}
