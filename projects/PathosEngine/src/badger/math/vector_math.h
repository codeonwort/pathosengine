#pragma once

#include "badger/types/vector_types.h"

namespace badger {

	// Given N (assuming normalized), returns two arbitrary vectors T and B to form an orthonormal basis.
	inline void calculateOrthonormalBasis(const vector3& N, vector3& outT, vector3& outB) {
		vector3 T(0.0f, 1.0f, 0.0f);

		// if almost parallel, choose another random direction
		float angle = glm::dot(N, T);
		if (fabs(angle) >= 0.999f) {
			T = vector3(1.0f, 0.0f, 0.0f);
		}

		outB = glm::normalize(glm::cross(N, T));
		outT = glm::cross(outB, N);
	}

}
