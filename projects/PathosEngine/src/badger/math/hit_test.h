#pragma once

#include "aabb.h"
#include "plane.h"
#include "badger/types/matrix_types.h"

// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html

namespace badger {

	inline AABB calculateWorldBounds(const AABB& localBounds, const matrix4& localToWorld) {
		vector3 minB = localBounds.minBounds;
		vector3 maxB = localBounds.maxBounds;
		vector3 vs[8];
		for (uint32 i = 0; i < 8; ++i) {
			vs[i].x = (i & 1) ? minB.x : maxB.x;
			vs[i].y = ((i >> 1) & 1) ? minB.y : maxB.y;
			vs[i].z = ((i >> 2) & 1) ? minB.z : maxB.z;
		}
		minB = vector3(std::numeric_limits<float>::max());
		maxB = vector3(std::numeric_limits<float>::lowest());
		for (uint32 i = 0; i < 8; ++i) {
			vs[i] = vector3(localToWorld * vector4(vs[i], 1.0f));
			minB = glm::min(minB, vs[i]);
			maxB = glm::max(maxB, vs[i]);
		}
		return AABB::fromMinMax(minB, maxB);
	}

	namespace hitTest {

		// Check if the AABB is on positive side of the plane.
		inline bool AABB_plane(const AABB& box, const Plane3D& plane) {
			float r = glm::dot(box.getHalfSize(), glm::abs(plane.normal));
			float s = plane.getSignedDistance(box.getCenter());
			//return std::abs(s) <= r; // intersection
			return -r <= s; // inside
		}

		// Check if the AABB is inside of the frustum.
		inline bool AABB_frustum(const AABB& box, const Frustum3D& frustum) {
			for (int32 i = 0; i < 6; ++i) {
				if (!AABB_plane(box, frustum.planes[i])) {
					return false;
				}
			}
			return true;
		}
		inline bool AABB_frustum_noFarPlane(const AABB& box, const Frustum3D& frustum) {
			for (int32 i = 0; i < 5; ++i) {
				if (!AABB_plane(box, frustum.planes[i])) {
					return false;
				}
			}
			return true;
		}

	}

}
