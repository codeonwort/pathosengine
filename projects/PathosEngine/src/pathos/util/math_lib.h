#pragma once

#include <glm/glm.hpp>

#undef min
#undef max

namespace pathos {

	// Given N (assuming normalized), returns two arbitrary vectors T and B to form an orthonormal basis.
	inline void calculateOrthonormalBasis(const glm::vec3& N, glm::vec3& outT, glm::vec3& outB) {
		glm::vec3 T(0.0f, 1.0f, 0.0f);

		// if almost parallel, choose another random direction
		float angle = glm::dot(N, T);
		if (fabs(angle) >= 0.999f) {
			T = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		outB = glm::normalize(glm::cross(N, T));
		outT = glm::cross(outB, N);
	}

	// #todo-refactoring: Duplicate to badger::min() and badger::max()
	template<typename T>
	T min(T x, T y) {
		return (x < y) ? x : y;
	}

	template<typename T>
	T max(T x, T y) {
		return (x < y) ? y : x;
	}

}
