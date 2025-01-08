#include "signed_volume.h"

namespace badger {

	static int32 compareSigns(float a, float b) {
		if (a > 0.0f && b > 0.0f) return 1;
		if (a < 0.0f && b < 0.0f) return 1;
		return 0;
	};

	vector3 projectPointOntoLine(const vector3& s1, const vector3& s2, const vector3& p) {
		vector3 ab = s2 - s1;
		vector3 ap = p - s1;
		vector3 p0 = s1 + ab * glm::dot(ab, ap) / glm::dot(ab, ab);
		return p0;
	}

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

	vector3 signedVolume2D(const vector3& s1, const vector3& s2, const vector3& s3) {
		vector3 normal = glm::cross(s2 - s1, s3 - s1);
		vector3 p0 = normal * glm::dot(s1, normal) / glm::dot(normal, normal);

		// Find the axis with the greatest projected area.
		int32 idx = 0;
		float maxArea = 0.0f;
		for (int32 i = 0; i < 3; ++i) {
			int32 j = (i + 1) % 3;
			int32 k = (i + 2) % 3;
			vector2 a(s1[j], s1[k]), b(s2[j], s2[k]), c(s3[j], s3[k]);
			vector2 ab = b - a, ac = c - a;

			float area = ab.x * ac.y - ab.y * ac.x;
			if (area * area > maxArea * maxArea) {
				idx = i;
				maxArea = area;
			}
		}

		// Project onto the appropriate axis.
		int32 x = (idx + 1) % 3;
		int32 y = (idx + 2) % 3;
		vector2 s[3] = {
			vector2(s1[x], s1[y]),
			vector2(s2[x], s2[y]),
			vector2(s3[x], s3[y])
		};
		vector2 p(p0[x], p0[y]);

		// Get the sub-areas of the triangles formed from the projected origin and the edges.
		vector3 areas;
		for (int32 i = 0; i < 3; ++i) {
			int32 j = (i + 1) % 3;
			int32 k = (i + 2) % 3;
			vector2 a = p, b = s[j], c = s[k];
			vector2 ab = b - a, ac = c - a;

			areas[i] = ab.x * ac.y - ab.y * ac.x;
		}

		// If the projected origin is inside the triangle, then return the barycentric points.
		if (compareSigns(maxArea, areas[0]) > 0 && compareSigns(maxArea, areas[1]) > 0 && compareSigns(maxArea, areas[2]) > 0) {
			vector3 lambdas = areas / maxArea;
			return lambdas;
		}

		// Project onto the edges and determine the closest point.
		float dist = std::numeric_limits<float>::max();
		vector3 lambdas(1.0f, 0.0f, 0.0f);
		for (int32 i = 0; i < 3; ++i) {
			int32 j = (i + 1) % 3;
			int32 k = (i + 2) % 3;

			vector3 edgePoints[3] = { s1, s2, s3 };
			vector2 lambdaEdge = signedVolume1D(edgePoints[j], edgePoints[k]);
			vector3 pt = edgePoints[j] * lambdaEdge[0] + edgePoints[k] * lambdaEdge[1];
			float lengthSq = glm::dot(pt, pt);
			if (lengthSq < dist) {
				dist = lengthSq;
				lambdas[i] = 0;
				lambdas[j] = lambdaEdge[0];
				lambdas[k] = lambdaEdge[1];
			}
		}

		return lambdas;
	}

	vector4 signedVolume3D(const vector3& s1, const vector3& s2, const vector3& s3, const vector3& s4) {
		matrix4 M;
		// The physics book uses row-major matrix and GLM matrix is column-major;
		// It's super confusing so I'm just using a setup that passes the unit tests...
#if 1
		M[0] = vector4(s1.x, s2.x, s3.x, s4.x);
		M[1] = vector4(s1.y, s2.y, s3.y, s4.y);
		M[2] = vector4(s1.z, s2.z, s3.z, s4.z);
		M[3] = vector4(1.0f, 1.0f, 1.0f, 1.0f);
#else
		M[0] = vector4(s1.x, s1.y, s1.z, 1.0f);
		M[1] = vector4(s2.x, s2.y, s2.z, 1.0f);
		M[2] = vector4(s3.x, s3.y, s3.z, 1.0f);
		M[3] = vector4(s4.x, s4.y, s4.z, 1.0f);
#endif

		auto getMinor = [](const matrix4& m, int32 row, int32 col) -> matrix3 {
			matrix3 minor;
#if 1
			int32 xx = 0;
			for (int32 x = 0; x < 4; ++x) {
				if (x == col) continue;
				int32 yy = 0;
				for (int32 y = 0; y < 4; ++y) {
					if (y == row) continue;
					minor[xx][yy] = m[x][y];
					++yy;
				}
				++xx;
			}
#else
			int32 yy = 0;
			for (int32 y = 0; y < 4; ++y) {
				if (y == row) continue;
				int32 xx = 0;
				for (int32 x = 0; x < 4; ++x) {
					if (x == col) continue;
					minor[yy][xx] = m[y][x];
					++xx;
				}
				++yy;
			}
#endif
			return minor;
			};

		auto cofactor = [&getMinor](const matrix4& m, int32 row, int32 col) -> float {
			matrix3 minor = getMinor(m, row, col);
			float C = (float)glm::pow(-1, row + 1 + col + 1) * glm::determinant(minor);
			return C;
			};

#if 1
		vector4 C4(cofactor(M, 0, 3), cofactor(M, 1, 3), cofactor(M, 2, 3), cofactor(M, 3, 3));
#else
		vector4 C4(cofactor(M, 3, 0), cofactor(M, 3, 1), cofactor(M, 3, 2), cofactor(M, 3, 3));
#endif
		float detM = C4[0] + C4[1] + C4[2] + C4[3];

		// If the barycentric coordinates put the origin inside the simplex, then return them.
		if (compareSigns(detM, C4[0]) > 0 && compareSigns(detM, C4[1]) > 0 &&
			compareSigns(detM, C4[2]) > 0 && compareSigns(detM, C4[3]) > 0) {
			vector4 lambdas = C4 * (1.0f / detM);
			return lambdas;
		}

		// Project the origin onto the faces and determine the closest one.
		vector4 lambdas;
		float dist = std::numeric_limits<float>::max();
		for (int32 i = 0; i < 4; ++i) {
			int32 j = (i + 1) % 4;
			int32 k = (i + 2) % 4;
			vector3 facePoints[4] = { s1, s2, s3, s4 };

			vector3 lambdasFace = signedVolume2D(facePoints[i], facePoints[j], facePoints[k]);
			vector3 pt = facePoints[i] * lambdasFace[0] + facePoints[j] * lambdasFace[1] + facePoints[k] * lambdasFace[2];
			float lengthSq = glm::dot(pt, pt);
			if (lengthSq < dist) {
				dist = lengthSq;
				lambdas = vector4(0.0f);
				lambdas[i] = lambdasFace[0];
				lambdas[j] = lambdasFace[1];
				lambdas[k] = lambdasFace[2];
			}
		}
		return lambdas;
	}

}
