#pragma once

#include <vector>

namespace badger {

	struct ConvexHullTriangle { size_t a, b, c; };

	void buildConvexHull(
		const std::vector<vector3>& vertices,
		std::vector<vector3>& outHullPoints,
		std::vector<ConvexHullTriangle>& outHullTriangles);

	vector3 calculateCenterOfMass(
		const std::vector<vector3>& points,
		const std::vector<ConvexHullTriangle>& triangles);

	matrix3 calculateInertiaTensor(
		const std::vector<vector3>& points,
		const std::vector<ConvexHullTriangle>& triangles,
		const vector3& centerOfMass);

}
