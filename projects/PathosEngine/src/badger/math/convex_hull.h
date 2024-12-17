#pragma once

#include <vector>

namespace badger {

	struct ConvexHullTriangle { size_t a, b, c; };

	void buildConvexHull(
		const std::vector<vector3>& vertices,
		std::vector<vector3>& outHullPoints,
		std::vector<ConvexHullTriangle>& outHullTriangles);

}
