#pragma once

#include "badger/types/vector_types.h"

struct Plane3D {
	static Plane3D fromNormalAndDistance(const vector3& n, float d) {
		return Plane3D{ n, d };
	}
	static Plane3D fromPointAndNormal(const vector3& p, const vector3& n) {
		// p: Random point on the plane
		return Plane3D{ n, glm::dot(p, n) };
	}
	// Surface normal rule: rotate from ab to ac
	static Plane3D fromThreePoints(const vector3& a, const vector3& b, const vector3& c) {
		return fromPointAndNormal(a, normalize(glm::cross(b - a, c - a)));
	}

	// < 0: Negative side
	// = 0: On plane
	// > 0: Positive side (where surface normal directs)
	inline float getSignedDistance(const vector3& p) const {
		//vector3 origin = normal * distance;
		//return glm::dot(p - origin, normal);
		return glm::dot(p, normal) - distance;
	}

	vector3 normal; // Surface normal
	float distance; // Length of perp vector from O to the plane.
};

struct Frustum3D {
	inline bool containsPoint(const vector3& p) const {
		for (int32 i = 0; i < 6; ++i) {
			if (planes[i].getSignedDistance(p) < 0.0f) {
				return false;
			}
			return true;
		}
	}

	// 0: top, 1: bottom, 2: left, 3: right, 4: near, 5: far
	Plane3D planes[6];
};
