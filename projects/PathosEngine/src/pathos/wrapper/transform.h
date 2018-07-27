#pragma once

#include <glm/glm.hpp>

namespace pathos {

	class Transform {
	protected:
		glm::mat4 matrix;
	public:
		Transform();
		Transform(const glm::mat4& matrix);
		const glm::mat4& getMatrix() const;
		inline void copyFrom(const glm::mat4& m) { matrix = m; }
		inline glm::vec3 getPosition() const { return glm::vec3(matrix[3]); }
		void identity();
		void append(const glm::mat4& t);
		void appendMove(const glm::vec3& movement);
		void appendMove(float dx, float dy, float dz);
		void appendRotation(float angle, const glm::vec3& axis);
		void appendScale(const glm::vec3& scale);
		void appendScale(float sx, float sy, float sz);
		inline void appendScale(float s) { appendScale(s, s, s); }
		void prepend(const glm::mat4& t);
		void prependMove(const glm::vec3& movement);
		void prependMove(float dx, float dy, float dz);
		void prependRotation(float angle, const glm::vec3& axis);
		void prependScale(const glm::vec3& scale);
		void prependScale(float sx, float sy, float sz);

		glm::vec3 transformVector(glm::vec3);
		glm::vec3 inverseTransformVector(glm::vec3) const;
		glm::vec3 transformPoint(glm::vec3);
		glm::vec3 inverseTransformPoint(glm::vec3);
	};

}