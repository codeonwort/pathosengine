#pragma once

#include <glm/glm.hpp>

namespace pathos {

	class Transform {
	protected:
		glm::mat4 matrix;
	public:
		Transform();
		const glm::mat4& getMatrix();
		inline void copyFrom(const glm::mat4& m) { matrix = m; }
		inline glm::vec3 getPosition() { return glm::vec3(matrix[3]); }
		void identity();
		void append(const glm::mat4& t);
		void appendMove(const glm::vec3& movement);
		void appendMove(float dx, float dy, float dz);
		void appendRotation(float angle, const glm::vec3& axis);
		glm::vec3 transformVector(glm::vec3);
		glm::vec3 inverseTransformVector(glm::vec3);
		glm::vec3 transformPoint(glm::vec3);
		glm::vec3 inverseTransformPoint(glm::vec3);
	};

}