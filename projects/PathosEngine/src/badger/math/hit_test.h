#pragma once

#include "aabb.h"
#include "plane.h"

// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html

namespace badger {

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
