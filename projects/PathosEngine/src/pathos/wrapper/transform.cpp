#include "pathos/wrapper/transform.h"
#include <glm/gtc/matrix_transform.hpp>

namespace pathos {

	Transform::Transform() :matrix(glm::mat4(1.0f)) {}
	Transform::Transform(const glm::mat4& _matrix) { matrix = _matrix; }

	const glm::mat4& Transform::getMatrix() const { return matrix; }

	void Transform::identity() { matrix = glm::mat4(1.0f); }

	void Transform::append(const glm::mat4& t) { matrix = t * matrix; }

	//void Transform::appendMove(const glm::vec3& movement) { matrix = glm::translate(matrix, movement); }
	void Transform::appendMove(const glm::vec3& movement) {
		matrix = glm::translate(glm::mat4(1.0f), movement) * matrix;
	}
	void Transform::appendMove(float dx, float dy, float dz) {
		appendMove(glm::vec3(dx, dy, dz));
	}

	void Transform::appendRotation(float angle, const glm::vec3& axis) {
		matrix = glm::rotate(glm::mat4(1.0f), angle, axis) * matrix;
	}

	void Transform::appendScale(const glm::vec3& scale) {
		matrix = glm::scale(glm::mat4(1.0f), scale) * matrix;
	}
	//void Transform::appendScale(const glm::vec3& scale) { matrix = glm::scale(matrix, scale); }
	void Transform::appendScale(float sx, float sy, float sz) {
		appendScale(glm::vec3(sx, sy, sz));
	}

	void Transform::prepend(const glm::mat4& t) {
		matrix = matrix * t;
	}

	void Transform::prependMove(const glm::vec3& movement) {
		matrix = glm::translate(matrix, movement);
	}

	void Transform::prependMove(float dx, float dy, float dz) {
		prependMove(glm::vec3(dx, dy, dz));
	}

	void Transform::prependRotation(float angle, const glm::vec3& axis) {
		matrix = glm::rotate(matrix, angle, axis);
	}

	void Transform::prependScale(const glm::vec3& scale) {
		matrix = glm::scale(matrix, scale);
	}

	void Transform::prependScale(float sx, float sy, float sz) {
		prependScale(glm::vec3(sx, sy, sz));
	}

	glm::vec3 Transform::transformVector(glm::vec3 v) {
		return glm::vec3(matrix * glm::vec4(v, 0.0f));
	}
	glm::vec3 Transform::inverseTransformVector(glm::vec3 v) const {
		return glm::vec3(glm::transpose(matrix) * glm::vec4(v, 0.0f));
	}

	glm::vec3 Transform::transformPoint(glm::vec3 v) {
		return glm::vec3(matrix * glm::vec4(v, 1.0f));
	}
	glm::vec3 Transform::inverseTransformPoint(glm::vec3 v) {
		return glm::vec3(glm::transpose(matrix) * glm::vec4(v, 1.0f));
	}

}