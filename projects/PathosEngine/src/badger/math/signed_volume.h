#pragma once

#include "badger/types/vector_types.h"
#include "badger/types/int_types.h"

namespace badger {

	// s1, s2: Two points on the line segment.
	// p: The point to project onto the line.
	vector3 projectPointOntoLine(const vector3& s1, const vector3& s2, const vector3& p) {
		vector3 ab = s2 - s1;
		vector3 ap = p - s1;
		vector3 p0 = s1 + ab * glm::dot(ab, ap) / glm::dot(ab, ab);
		return p0;
	}

	// s1, s2: End points of line segement.
	// 
	// Returns barycentric coordinates of the origin projected onto line segment [s1, s2].
	// Denote that projected point as p, the return value as (k1, k2), then p = k1*s1 + k2*s2.
	vector2 signedVolume1D(const vector3& s1, const vector3& s2) {
		// #todo-physics: Original book assumes the input is always origin but
		// it seems this function could be generalized to take it as a parameter.
		vector3 p0 = projectPointOntoLine(s1, s2, vector3(0.0f));

		// Choose the axis with the greatest length.
		int32 idx = 0;
		float maxLen = 0.0f;
		for (int32 i = 0; i < 3; ++i) {
			float len = s2[i] - s1[i];
			if (len * len > maxLen * maxLen) {
				maxLen = len;
				idx = i;
			}
		}

		// Project the simplex points and projected origin onto the axis with greatest length.
		float a = s1[idx], b = s2[idx], p = p0[idx];

		// Signed distance from a to p and from p to b.
		float C1 = p - a, C2 = b - p;

		// If p is between [a, b]
		if ((a < p && p < b) || (b < p && p < a)) {
			vector2 lambdas(C2 / maxLen, C1 / maxLen);
			return lambdas;
		}

		// If p is on the far side of a
		if ((a <= b && p <= a) || (a >= b && p >= a)) {
			return vector2(1.0f, 0.0f);
		}

		// p is on the far side of b
		return vector2(0.0f, 1.0f);
	}

}
