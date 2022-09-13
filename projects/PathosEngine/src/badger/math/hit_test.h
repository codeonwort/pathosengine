#pragma once

#include "aabb.h"
#include "plane.h"

// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html

namespace badger {

	namespace hitTest {

		inline bool AABB_plane(const AABB& box, const Plane3D& plane) {
			float r = glm::dot(box.getHalfSize(), glm::abs(plane.normal));
			float s = plane.getSignedDistance(box.getCenter());
			return std::abs(s) <= r;
		}

		inline bool AABB_frustum(const AABB& box, const Frustum3D& frustum) {
			for (int32 i = 0; i < 6; ++i) {
				if (!AABB_plane(box, frustum.planes[i])) {
					return false;
				}
			}
			return true;
		}

	}

}
