#include "pathos/wrapper/transform.h"
#include <glm/gtc/matrix_transform.hpp>

namespace pathos {

	Transform::Transform() :matrix(glm::mat4(1.0f)) {}
	Transform::Transform(const glm::mat4& _matrix) { matrix = _matrix; }

	const glm::mat4& Transform::getMatrix() const { return matrix; }

	void Transform::identity() { matrix = glm::mat4(1.0f); }
	void Transform::append(const glm::mat4& t) { matrix = matrix * t; }
	void Transform::appendMove(const glm::vec3& movement) { matrix = glm::translate(matrix, movement); }
	void Transform::appendMove(float dx, float dy, float dz) { appendMove(glm::vec3(dx, dy, dz)); }
	void Transform::appendRotation(float angle, const glm::vec3& axis) { matrix = glm::rotate(matrix, angle, axis); }
	void Transform::appendScale(const glm::vec3& scale) { matrix = glm::scale(matrix, scale); }
	void Transform::appendScale(float sx, float sy, float sz) { matrix = glm::scale(matrix, glm::vec3(sx, sy, sz)); }
	glm::vec3 Transform::transformVector(glm::vec3 v) { return glm::vec3(matrix * glm::vec4(v, 0.0f)); }
	glm::vec3 Transform::inverseTransformVector(glm::vec3 v) { return glm::vec3(glm::transpose(matrix) * glm::vec4(v, 0.0f)); }
	glm::vec3 Transform::transformPoint(glm::vec3 v) { return glm::vec3(matrix * glm::vec4(v, 1.0f)); }
	glm::vec3 Transform::inverseTransformPoint(glm::vec3 v) { return glm::vec3(glm::transpose(matrix) * glm::vec4(v, 1.0f)); }

}