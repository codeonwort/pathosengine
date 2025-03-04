#include "convex_hull.h"
#include "aabb.h"

namespace badger {

	// #todo-physics: It seems lots of util functions here are O(NN) when they can be O(N).

	static vector3 safeNormalize(const vector3& v) {
		if (v == vector3(0.0f)) return v;
		return glm::normalize(v);
	}

	// Returns array index of the point.
	static size_t findFurthestPointInDir(const std::vector<vector3>& points, const vector3& dir) {
		size_t ix = 0;
		float maxDist = glm::dot(dir, points[0]);
		for (auto i = 1u; i < points.size(); ++i) {
			float dist = glm::dot(dir, points[i]);
			if (dist > maxDist) {
				maxDist = dist;
				ix = i;
			}
		}
		return ix;
	}

	// a, b : two points on the line.
	// pt : a point we want to get the distance to line ab.
	static float distanceFromLine(const vector3& a, const vector3& b, const vector3& pt) {
		vector3 ab = safeNormalize(b - a);

		vector3 ray = pt - a;
		vector3 projection = ab * glm::dot(ray, ab);
		vector3 perp = ray - projection;
		return glm::length(perp);
	}

	// a, b : two points on the line.
	static vector3 findFurthestPointFromLine(const std::vector<vector3>& points, const vector3& a, const vector3& b) {
		size_t ix = 0;
		float maxDist = distanceFromLine(a, b, points[0]);
		for (size_t i = 1u; i < points.size(); ++i) {
			float dist = distanceFromLine(a, b, points[i]);
			if (dist > maxDist) {
				maxDist = dist;
				ix = i;
			}
		}
		return points[ix];
	}

	// a, b, c : vertices of the triangle.
	static float distanceFromTriangle(const vector3& a, const vector3& b, const vector3& c, const vector3& pt) {
		vector3 ab = b - a;
		vector3 ac = c - a;
		vector3 normal = safeNormalize(glm::cross(ab, ac));

		vector3 ray = pt - a;
		float dist = glm::dot(ray, normal);
		return dist;
	}

	static vector3 findFurthestPointFromTriangle(const std::vector<vector3>& points, const vector3& a, const vector3& b, const vector3& c) {
		size_t ix = 0;
		float maxDist = distanceFromTriangle(a, b, c, points[0]);
		for (size_t i = 1u; i < points.size(); ++i) {
			float dist = distanceFromTriangle(a, b, c, points[i]);
			if (dist > maxDist) {
				maxDist = dist;
				ix = i;
			}
		}
		return points[ix];
	}

	static void buildTetrahedron(
		const std::vector<vector3>& vertices,
		std::vector<vector3>& outHullPoints,
		std::vector<ConvexHullTriangle>& outHullTriangles)
	{
		outHullPoints.clear();
		outHullTriangles.clear();

		vector3 points[4];

		size_t ix = findFurthestPointInDir(vertices, vector3(1.0f, 0.0f, 0.0f));
		points[0] = vertices[ix];
		ix = findFurthestPointInDir(vertices, -points[0]);
		points[1] = vertices[ix];
		points[2] = findFurthestPointFromLine(vertices, points[0], points[1]);
		points[3] = findFurthestPointFromTriangle(vertices, points[0], points[1], points[2]);

		// Ensure ordering is CCW.
		float dist = distanceFromTriangle(points[0], points[1], points[2], points[3]);
		if (dist > 0.0f) {
			std::swap(points[0], points[1]);
		}

		outHullPoints.emplace_back(points[0]);
		outHullPoints.emplace_back(points[1]);
		outHullPoints.emplace_back(points[2]);
		outHullPoints.emplace_back(points[3]);
		outHullTriangles.emplace_back(ConvexHullTriangle{ 0, 1, 2 });
		outHullTriangles.emplace_back(ConvexHullTriangle{ 0, 2, 3 });
		outHullTriangles.emplace_back(ConvexHullTriangle{ 2, 1, 3 });
		outHullTriangles.emplace_back(ConvexHullTriangle{ 1, 0, 3 });
	}

	static void removeInternalPoints(
		const std::vector<vector3>& hullPoints,
		const std::vector<ConvexHullTriangle>& hullTriangles,
		std::vector<vector3>& outCheckPoints)
	{
		for (size_t i = 0u; i < outCheckPoints.size(); ++i) {
			const vector3& pt = outCheckPoints[i];

			bool isExternal = false;
			for (size_t t = 0u; t < hullTriangles.size(); ++t) {
				const ConvexHullTriangle& tri = hullTriangles[t];
				const vector3& a = hullPoints[tri.a];
				const vector3& b = hullPoints[tri.b];
				const vector3& c = hullPoints[tri.c];

				// If the point is in front of any triangle then it's external.
				if (distanceFromTriangle(a, b, c, pt) > 0.0f) {
					isExternal = true;
					break;
				}
			}
			// If it's not external, then it's inside the polyhderon and should be removed.
			if (!isExternal) {
				outCheckPoints.erase(outCheckPoints.begin() + i);
				--i;
			}
		}
		// Also remove any points that are a little too close to the hull points.
		for (size_t i = 0u; i < outCheckPoints.size(); ++i) {
			const vector3& pt = outCheckPoints[i];

			bool tooClose = false;
			for (size_t j = 0u; j < hullPoints.size(); ++j) {
				vector3 ray = hullPoints[j] - pt;
				if (glm::dot(ray, ray) < 0.01f * 0.01f) {
					tooClose = true;
					break;
				}
			}
			if (tooClose) {
				outCheckPoints.erase(outCheckPoints.begin() + i);
				--i;
			}
		}
	}

	// Compare the incoming edge with all the edges in the facing tris, then return true if it's unique.
	static bool isEdgeUnique(const std::vector<ConvexHullTriangle>& triangles,
		const std::vector<size_t>& facingTriangles, size_t ignoreTriangle, const Edge& edge)
	{
		for (size_t triIx : facingTriangles) {
			if (ignoreTriangle == triIx) continue;

			const ConvexHullTriangle& tri = triangles[triIx];
			Edge edges[3] = { { tri.a, tri.b }, { tri.b, tri.c }, { tri.c, tri.a } };
			for (size_t e = 0; e < 3; ++e) {
				if (edge == edges[e]) return false;
			}
		}
		return true;
	}

	static void addPoint(const vector3& pt,
		std::vector<vector3>& inoutHullPoints,
		std::vector<ConvexHullTriangle>& inoutHullTriangles)
	{
		CHECK(inoutHullTriangles.size() > 1);

		// Find all triangles that face this point.
		std::vector<size_t> facingTriangles;
		for (int32 i = (int32)(inoutHullTriangles.size() - 1); i >= 0; --i) {
			const ConvexHullTriangle& tri = inoutHullTriangles[i];
			const vector3& a = inoutHullPoints[tri.a];
			const vector3& b = inoutHullPoints[tri.b];
			const vector3& c = inoutHullPoints[tri.c];
			if (distanceFromTriangle(a, b, c, pt) > 0.0f) {
				facingTriangles.push_back(i);
			}
		}

		// Find all edges that are unique to the triangles.
		std::vector<Edge> uniqueEdges;
		for (size_t triIx : facingTriangles) {
			const ConvexHullTriangle& tri = inoutHullTriangles[triIx];
			Edge edges[3] = { { tri.a, tri.b }, { tri.b, tri.c }, { tri.c, tri.a } };
			for (size_t e = 0; e < 3; ++e) {
				if (isEdgeUnique(inoutHullTriangles, facingTriangles, triIx, edges[e])) {
					uniqueEdges.push_back(edges[e]);
				}
			}
		}

		// Remove old facing triangles.
		for (size_t triIx : facingTriangles) {
			inoutHullTriangles.erase(inoutHullTriangles.begin() + triIx);
		}

		// Add the new point.
		inoutHullPoints.push_back(pt);
		size_t newIx = inoutHullPoints.size() - 1;

		// Add triangles for each unique edge.
		for (const Edge& e : uniqueEdges) {
			ConvexHullTriangle tri{ e.a, e.b, newIx };
			inoutHullTriangles.emplace_back(tri);
		}
	}

	static void removeUnreferencedVertices(
		std::vector<vector3>& inoutHullPoints,
		std::vector<ConvexHullTriangle>& inoutHullTriangles)
	{
		for (size_t i = 0u; i < inoutHullPoints.size(); ++i) {
			bool isUsed = false;
			for (size_t j = 0u; j < inoutHullTriangles.size(); ++j) {
				const ConvexHullTriangle& tri = inoutHullTriangles[j];
				if (tri.a == i || tri.b == i || tri.c == i) {
					isUsed = true;
					break;
				}
			}
			if (isUsed) continue;

			for (size_t j = 0u; j < inoutHullTriangles.size(); ++j) {
				ConvexHullTriangle& tri = inoutHullTriangles[j];
				if (tri.a > i) --tri.a;
				if (tri.b > i) --tri.b;
				if (tri.c > i) --tri.c;
			}
			inoutHullPoints.erase(inoutHullPoints.begin() + i);
			--i;
		}
	}

	static void expandConvexHull(
		const std::vector<vector3>& vertices,
		std::vector<vector3>& inoutHullPoints,
		std::vector<ConvexHullTriangle>& inoutHullTriangles)
	{
		auto externalVertices = vertices;
		removeInternalPoints(inoutHullPoints, inoutHullTriangles, externalVertices);

		while (externalVertices.size() > 0) {
			size_t ix = findFurthestPointInDir(externalVertices, externalVertices[0]);
			vector3 pt = externalVertices[ix];

			externalVertices.erase(externalVertices.begin() + ix);

			addPoint(pt, inoutHullPoints, inoutHullTriangles);
			removeInternalPoints(inoutHullPoints, inoutHullTriangles, externalVertices);
		}
		removeUnreferencedVertices(inoutHullPoints, inoutHullTriangles);
	}

	void buildConvexHull(
		const std::vector<vector3>& vertices,
		std::vector<vector3>& outHullPoints,
		std::vector<ConvexHullTriangle>& outHullTriangles)
	{
		if (vertices.size() < 4) return;

		buildTetrahedron(vertices, outHullPoints, outHullTriangles);
		expandConvexHull(vertices, outHullPoints, outHullTriangles);
	}

}

namespace badger {
	
	static bool isExternal(
		const std::vector<vector3>& points,
		const std::vector<ConvexHullTriangle>& triangles,
		const vector3& pt)
	{
		bool bExternal = false;
		for (const ConvexHullTriangle& tri : triangles) {
			const vector3& a = points[tri.a];
			const vector3& b = points[tri.b];
			const vector3& c = points[tri.c];

			if (distanceFromTriangle(a, b, c, pt) > 0.0f) {
				bExternal = true;
				break;
			}
		}

		return bExternal;
	}

	static AABB buildAABB(const std::vector<vector3>& points) {
		CHECK(points.size() > 0);
		AABB aabb = AABB::fromMinMax(points[0], points[0]);
		if (points.size() > 1) {
			for (auto it = points.begin() + 1; it != points.end(); ++it) {
				aabb.expand(*it);
			}
		}
		return aabb;
	}

	vector3 calculateCenterOfMass(
		const std::vector<vector3>& points,
		const std::vector<ConvexHullTriangle>& triangles)
	{
		const size_t numSamples = 100; // #todo-physics: Hard-coded iteration count.

		AABB bounds = buildAABB(points);

		vector3 cm(0.0f);
		const float dx = bounds.getSize().x / (float)numSamples;
		const float dy = bounds.getSize().y / (float)numSamples;
		const float dz = bounds.getSize().z / (float)numSamples;

		size_t sampleCount = 0;
		for (float x = bounds.minBounds.x; x < bounds.maxBounds.x; x += dx) {
			for (float y = bounds.minBounds.y; y < bounds.maxBounds.y; y += dy) {
				for (float z = bounds.minBounds.z; z < bounds.maxBounds.z; z += dz) {
					vector3 pt(x, y, z);
					if (isExternal(points, triangles, pt)) {
						continue;
					}
					cm += pt;
					sampleCount++;
				}
			}
		}
		CHECK(sampleCount > 0);
		cm /= (float)sampleCount;
		return cm;
	}

	matrix3 calculateInertiaTensor(
		const std::vector<vector3>& points,
		const std::vector<ConvexHullTriangle>& triangles,
		const vector3& centerOfMass)
	{
		const size_t numSamples = 100; // #todo-physics: Hard-coded iteration count.

		AABB bounds = buildAABB(points);

		matrix3 tensor(0.0f);
		const float dx = bounds.getSize().x / (float)numSamples;
		const float dy = bounds.getSize().y / (float)numSamples;
		const float dz = bounds.getSize().z / (float)numSamples;

		size_t sampleCount = 0;
		for (float x = bounds.minBounds.x; x < bounds.maxBounds.x; x += dx) {
			for (float y = bounds.minBounds.y; y < bounds.maxBounds.y; y += dy) {
				for (float z = bounds.minBounds.z; z < bounds.maxBounds.z; z += dz) {
					vector3 pt(x, y, z);
					if (isExternal(points, triangles, pt)) {
						continue;
					}
					
					pt -= centerOfMass;

					tensor[0] += vector3(pt.y * pt.y + pt.z * pt.z, -pt.x * pt.y, -pt.x * pt.z);
					tensor[1] += vector3(-pt.x * pt.y, pt.z * pt.z + pt.x * pt.x, -pt.y * pt.z);
					tensor[2] += vector3(-pt.x * pt.z, -pt.y * pt.z, pt.x * pt.x + pt.y * pt.y);

					sampleCount++;
				}
			}
		}

		tensor *= 1.0f / (float)sampleCount;
		return tensor;
	}

}
