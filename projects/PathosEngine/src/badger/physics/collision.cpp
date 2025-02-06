#include "collision.h"
#include "shape.h"
#include "badger/math/signed_volume.h"
#include "badger/math/convex_hull.h"
#include "badger/math/vector_math.h"

#include <algorithm>

namespace badger {
	namespace physics {

		struct PseudoBody {
			int32 id;
			float value;
			bool isMin;
		};

		static vector3 safeNormalize(const vector3& v) {
			if (v == vector3(0.0f)) return v;
			return glm::normalize(v);
		}

		static bool raySphere(
			const vector3& rayStart, const vector3& rayDir,
			const vector3& sphereCenter, float sphereRadius,
			float& t1, float& t2)
		{
			vector3 m = sphereCenter - rayStart;
			float a = glm::dot(rayDir, rayDir);
			float b = glm::dot(m, rayDir);
			float c = glm::dot(m, m) - sphereRadius * sphereRadius;

			float delta = b * b - a * c;
			float invA = 1.0f / a;

			if (delta < 0.0f) return false; // No real solutions exist

			float deltaRoot = sqrtf(delta);
			t1 = invA * (b - deltaRoot);
			t2 = invA * (b + deltaRoot);

			return true;
		}

		static bool sphereSphereDynamic(
			const ShapeSphere* shapeA, const ShapeSphere* shapeB,
			const vector3& posA, const vector3& posB,
			const vector3& velA, const vector3& velB,
			float dt,
			vector3& surfaceA, vector3& surfaceB, float& toi)
		{
			const vector3 relVel = velA - velB;
			const vector3 startA = posA;
			const vector3 endA = posA + relVel * dt;
			const vector3 rayDir = endA - startA;

			float t0 = 0.0f, t1 = 0.0f;
			if (glm::dot(rayDir, rayDir) < 0.001f * 0.001f) {
				// Ray is too short, just check if already intersecting
				vector3 ab = posB - posA;
				float radius = shapeA->getRadius() + shapeB->getRadius() + 0.001f;
				if (glm::dot(ab, ab) > radius * radius) {
					return false;
				}
			} else if (!raySphere(posA, rayDir, posB, shapeA->getRadius() + shapeB->getRadius(), t0, t1)) {
				return false;
			}

			// [0, 1] -> [0, dt]
			t0 *= dt; t1 *= dt;

			// If the collision is only in the past, then there's not future collision this frame
			if (t1 < 0.0f) return false;

			toi = t0 < 0.0f ? 0.0f : t0;

			// If the earliest collision is too far in the future, then there's no collision this frame
			if (toi > dt) return false;

			vector3 newPosA = posA + velA * toi;
			vector3 newPosB = posB + velB * toi;
			vector3 ab = safeNormalize(newPosB - newPosA);

			surfaceA = newPosA + ab * shapeA->getRadius();
			surfaceB = newPosB - ab * shapeB->getRadius();
			return true;
		}

		static bool sphereSphereStatic(
			const ShapeSphere* sphereA, const ShapeSphere* sphereB,
			const vector3& posA, const vector3& posB,
			vector3& ptOnA, vector3& ptOnB)
		{
			const vector3 ab = posB - posA;
			vector3 norm = safeNormalize(ab);

			ptOnA = posA + norm * sphereA->getRadius();
			ptOnB = posB - norm * sphereB->getRadius();

			const float radiusAB = sphereA->getRadius() + sphereB->getRadius();
			const float lengthSq = glm::dot(ab, ab);
			return lengthSq <= (radiusAB * radiusAB);
		}

		bool intersect(Body* bodyA, Body* bodyB, float dt, Contact& outContact) {
			outContact.bodyA = bodyA;
			outContact.bodyB = bodyB;

			if (bodyA->getShape()->getType() == Shape::EShapeType::Sphere && bodyB->getShape()->getType() == Shape::EShapeType::Sphere) {
				const ShapeSphere* sphereA = (const ShapeSphere*)bodyA->getShape();
				const ShapeSphere* sphereB = (const ShapeSphere*)bodyB->getShape();

				vector3 posA = bodyA->getPosition();
				vector3 velA = bodyA->getLinearVelocity();
				vector3 posB = bodyB->getPosition();
				vector3 velB = bodyB->getLinearVelocity();

				if (sphereSphereDynamic(sphereA, sphereB, posA, posB, velA, velB, dt,
					outContact.surfaceA_WS, outContact.surfaceB_WS, outContact.timeOfImpact))
				{
					// Step bodies forward to get local space collision points
					bodyA->update(outContact.timeOfImpact);
					bodyB->update(outContact.timeOfImpact);

					outContact.surfaceA_LS = bodyA->worldSpaceToBodySpace(outContact.surfaceA_WS);
					outContact.surfaceB_LS = bodyB->worldSpaceToBodySpace(outContact.surfaceB_WS);

					outContact.normal = safeNormalize(bodyA->getPosition() - bodyB->getPosition());

					// Unwind time step
					bodyA->update(-outContact.timeOfImpact);
					bodyB->update(-outContact.timeOfImpact);

					vector3 ab = bodyB->getPosition() - bodyA->getPosition();
					float r = glm::length(ab) - (sphereA->getRadius() + sphereB->getRadius());

					outContact.separationDistance = r;
					return true;
				}
			}

			return false;
		}

		static void sortBodyBounds(std::vector<Body*>& bodies, std::vector<PseudoBody>& sortedArray, float deltaSeconds) {
			auto compareSAP = [](const PseudoBody& a, const PseudoBody& b) {
				return a.value < b.value;
			};
			vector3 axis = glm::normalize(vector3(1.0f, 1.0f, 1.0f));

			sortedArray.resize(bodies.size() * 2u);
			for (auto i = 0u; i < bodies.size(); ++i) {
				const Body* body = bodies[i];
				AABB bounds = body->getShape()->getBounds(body->getPosition(), body->getOrientation());

				vector3 mov = body->getLinearVelocity() * deltaSeconds;
				bounds = bounds + AABB::fromMinMax(bounds.minBounds + mov, bounds.maxBounds + mov);

				const vector3 EPS(0.01f);
				bounds = bounds + AABB::fromMinMax(bounds.minBounds - EPS, bounds.maxBounds + EPS);

				sortedArray[i * 2 + 0].id = (int32)i;
				sortedArray[i * 2 + 0].value = glm::dot(axis, bounds.minBounds);
				sortedArray[i * 2 + 0].isMin = true;

				sortedArray[i * 2 + 1].id = (int32)i;
				sortedArray[i * 2 + 1].value = glm::dot(axis, bounds.maxBounds);
				sortedArray[i * 2 + 1].isMin = false;
			}

			std::sort(sortedArray.begin(), sortedArray.end(), compareSAP);
		}

		static void buildPairs(std::vector<CollisionPair>& collisionPairs, const std::vector<PseudoBody>& sortedBodies) {
			collisionPairs.clear();
			collisionPairs.reserve(sortedBodies.size() * sortedBodies.size() / 4);

			for (auto i = 0u; i < sortedBodies.size(); ++i) {
				const PseudoBody& a = sortedBodies[i];
				if (!a.isMin) continue;

				CollisionPair pair;
				pair.a = a.id;

				for (int j = i + 1; j < sortedBodies.size(); ++j) {
					const PseudoBody& b = sortedBodies[j];
					if (b.id == a.id) break;
					if (b.isMin == false) continue;

					pair.b = b.id;
					collisionPairs.emplace_back(pair);
				}
			}
		}

		static void sweepAndPrune1D(std::vector<Body*>& bodies, std::vector<CollisionPair>& outPairs, float deltaSeconds) {
			std::vector<PseudoBody> sortedBodies;
			sortBodyBounds(bodies, sortedBodies, deltaSeconds);
			buildPairs(outPairs, sortedBodies);
		}

		void broadPhase(std::vector<Body*>& bodies, std::vector<CollisionPair>& outPairs, float deltaSeconds) {
			sweepAndPrune1D(bodies, outPairs, deltaSeconds);
		}

	}
}

// Simplex
namespace badger {
	namespace physics {

		struct SupportPoint {
			vector3 xyz = vector3(0.0f); // Point on the minkowski sum
			vector3 ptA = vector3(0.0f); // Point on bodyA
			vector3 ptB = vector3(0.0f); // Point on bodyB
		};

		static SupportPoint support(const Body* bodyA, const Body* bodyB, vector3 dir, float bias) {
			dir = glm::normalize(dir);

			SupportPoint point;
			point.ptA = bodyA->getShape()->support(dir, bodyA->getPosition(), bodyA->getOrientation(), bias);
			point.ptB = bodyB->getShape()->support(-dir, bodyB->getPosition(), bodyB->getOrientation(), bias);
			point.xyz = point.ptA - point.ptB; // Support in the minkowski sum (A + (-B))

			return point;
		}

		static bool simplexSignedVolumes(SupportPoint* points, int32 n, vector3& outNewDir, vector4& outLambdas) {
			const float EPSILON = 0.0001f * 0.0001f;
			outLambdas = vector4(0.0f);

			bool bIntersects = false;
			switch (n) {
				case 2: {
					vector2 lambdas = badger::signedVolume1D(points[0].xyz, points[1].xyz);
					vector3 v(0.0f);
					for (int32 i = 0; i < 2; ++i) v += points[i].xyz * lambdas[i];
					outNewDir = -v;
					bIntersects = glm::dot(v, v) < EPSILON;
					outLambdas[0] = lambdas[0];
					outLambdas[1] = lambdas[1];
					break;
				}
				case 3: {
					vector3 lambdas = badger::signedVolume2D(points[0].xyz, points[1].xyz, points[2].xyz);
					vector3 v(0.0f);
					for (int32 i = 0; i < 3; ++i) v += points[i].xyz * lambdas[i];
					outNewDir = -v;
					bIntersects = glm::dot(v, v) < EPSILON;
					outLambdas[0] = lambdas[0];
					outLambdas[1] = lambdas[1];
					outLambdas[2] = lambdas[2];
					break;
				}
				case 4: {
					vector4 lambdas = badger::signedVolume3D(points[0].xyz, points[1].xyz, points[2].xyz, points[3].xyz);
					vector3 v(0.0f);
					for (int32 i = 0; i < 4; ++i) v += points[i].xyz * lambdas[i];
					outNewDir = -v;
					bIntersects = glm::dot(v, v) < EPSILON;
					outLambdas[0] = lambdas[0];
					outLambdas[1] = lambdas[1];
					outLambdas[2] = lambdas[2];
					outLambdas[3] = lambdas[3];
					break;
				}
			}

			return bIntersects;
		}

		static bool simplexContainsPoint(const SupportPoint simplexPoints[4], const SupportPoint& newPt) {
			const float PRECISION = 1e-6f;
			for (int32 i = 0; i < 4; ++i) {
				vector3 delta = simplexPoints[i].xyz - newPt.xyz;
				if (glm::dot(delta, delta) < PRECISION * PRECISION) {
					return true;
				}
			}
			return false;
		}

		static void sortValids(SupportPoint simplexPoints[4], vector4& lambdas) {
			vector4 validLambdas(0.0f);
			int32 validCount = 0;
			SupportPoint validPoints[4];
			for (int32 i = 0; i < 4; ++i) {
				if (lambdas[i] != 0.0f) {
					validPoints[validCount] = simplexPoints[i];
					validLambdas[validCount] = lambdas[i];
					validCount++;
				}
			}

			for (int32 i = 0; i < 4; ++i) {
				simplexPoints[i] = validPoints[i];
				lambdas[i] = validLambdas[i];
			}
		}

		static int32 numValids(const vector4& lambdas) {
			int32 n = 0;
			for (int32 i = 0; i < 4; ++i) {
				if (lambdas[i] != 0.0f) n++;
			}
			return n;
		}

	}
}

// Expanding Polytope Algorithm
namespace badger {
	namespace physics {

		// Returns barycentricCoords of pt on the triangle (s1, s2, s3).
		static vector3 barycentricCoordinates(vector3 s1, vector3 s2, vector3 s3, const vector3& pt) {
			s1 = s1 - pt;
			s2 = s2 - pt;
			s3 = s3 - pt;

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
				vector2(s1[x], s1[y]), vector2(s2[x], s2[y]), vector2(s3[x], s3[y])
			};
			vector2 p = vector2(p0[x], p0[y]);

			// Get the sub-areas of the triangles formed from the projected origin and the edges.
			vector3 areas;
			for (int32 i = 0; i < 3; ++i) {
				int32 j = (i + 1) % 3;
				int32 k = (i + 2) % 3;

				vector2 a = p, b = s[j], c = s[k];
				vector2 ab = b - a, ac = c - a;

				areas[i] = ab.x * ac.y - ab.y * ac.x;
			}

			vector3 lambdas = areas / maxArea;
			if (glm::any(glm::isnan(lambdas)) || glm::any(glm::isinf(lambdas))) {
				lambdas = vector3(1.0f, 0.0f, 0.0f);
			}
			return lambdas;
		}

		static vector3 normalDirection(const ConvexHullTriangle& tri, const std::vector<SupportPoint>& points) {
			const vector3& a = points[tri.a].xyz;
			const vector3& b = points[tri.b].xyz;
			const vector3& c = points[tri.c].xyz;

			vector3 ab = b - a, ac = c - a;
			vector3 normal = safeNormalize(glm::cross(ab, ac));
			return normal;
		}

		static float signedDistanceToTriangle(const ConvexHullTriangle& tri, const vector3& pt, const std::vector<SupportPoint>& points) {
			const vector3 normal = normalDirection(tri, points);
			const vector3& a = points[tri.a].xyz;
			const vector3 a2pt = pt - a;
			const float dist = glm::dot(normal, a2pt);
			return dist;
		}

		static int32 closestTriangle(const std::vector<ConvexHullTriangle>& triangles, const std::vector<SupportPoint>& points) {
			float minDistSq = FLT_MAX;
			int32 idx = -1, n = (int32)triangles.size();
			for (int32 i = 0; i < n; ++i) {
				const ConvexHullTriangle& tri = triangles[i];
				float dist = signedDistanceToTriangle(tri, vector3(0.0f), points);
				float distSq = dist * dist;
				if (distSq < minDistSq) {
					idx = i;
					minDistSq = distSq;
				}
			}
			return idx;
		}

		static bool hasPoint(const vector3& w, const std::vector<ConvexHullTriangle>& triangles, const std::vector<SupportPoint>& points) {
			const float epsilons = 0.001f * 0.001f;
			vector3 delta;

			for (size_t i = 0u; i < triangles.size(); ++i) {
				const ConvexHullTriangle& tri = triangles[i];
				delta = w - points[tri.a].xyz;
				if (glm::dot(delta, delta) < epsilons) return true;
				delta = w - points[tri.b].xyz;
				if (glm::dot(delta, delta) < epsilons) return true;
				delta = w - points[tri.c].xyz;
				if (glm::dot(delta, delta) < epsilons) return true;
			}
			return false;
		}

		static int32 removeTrianglesFacingPoint(const vector3& pt, std::vector<ConvexHullTriangle>& triangles, const std::vector<SupportPoint>& points) {
			int32 numRemoved = 0;
			for (size_t i = 0u; i < triangles.size(); ++i) {
				const ConvexHullTriangle& tri = triangles[i];
				float dist = signedDistanceToTriangle(tri, pt, points);
				if (dist > 0.0f) {
					// This triangle faces the point. Remove it.
					triangles.erase(triangles.begin() + i);
					--i;
					++numRemoved;
				}
			}
			return numRemoved;
		}

		static void findDanglingEdges(std::vector<Edge>& danglingEdges, const std::vector<ConvexHullTriangle>& triangles) {
			danglingEdges.clear();

			for (size_t i = 0u; i < triangles.size(); ++i) {
				const ConvexHullTriangle& tri = triangles[i];

				Edge edges[3] = { { tri.a, tri.b }, {tri.b, tri.c}, {tri.c, tri.a} };
				int32 counts[3] = { 0, 0, 0 };
				for (size_t j = 0u; j < triangles.size(); ++j) {
					if (j == i) continue;
					const ConvexHullTriangle& tri2 = triangles[j];
					Edge edges2[3] = { {tri2.a, tri2.b}, {tri2.b, tri2.c}, {tri2.c, tri2.a} };
					for (int32 k = 0; k < 3; ++k) {
						if (edges[k] == edges2[0]) counts[k] += 1;
						if (edges[k] == edges2[1]) counts[k] += 1;
						if (edges[k] == edges2[2]) counts[k] += 1;
					}
				}
				// An edge that isn't shared is dangling.
				for (int32 k = 0; k < 3; ++k) {
					if (0 == counts[k]) {
						danglingEdges.push_back(edges[k]);
					}
				}
			}
		}

		static float expandingPolytopeAlgorithm(
			const Body* bodyA, const Body* bodyB, float bias, const SupportPoint simplexPoints[4],
			vector3& ptOnA, vector3& ptOnB)
		{
			std::vector<SupportPoint> points;
			std::vector<ConvexHullTriangle> triangles;
			std::vector<Edge> danglingEdges;

			vector3 center(0.0f);
			for (size_t i = 0; i < 4; ++i) {
				points.push_back(simplexPoints[i]);
				center += simplexPoints[i].xyz;
			}
			center *= 0.25f;

			// Build the triangles.
			for (size_t i = 0; i < 4; ++i) {
				size_t j = (i + 1) % 4;
				size_t k = (i + 2) % 4;
				ConvexHullTriangle tri{ i, j, k };

				size_t unusedPt = (i + 3) % 4;
				float dist = signedDistanceToTriangle(tri, points[unusedPt].xyz, points);

				// The unused point is always on the negative/inside of the triangle.
				// Make sure the normal points away.
				if (dist > 0.0f) {
					std::swap(tri.a, tri.b);
				}

				triangles.emplace_back(tri);
			}

			// Expand the simplex to find the closest face of the CSO to the origin.
			while (true) {
				const int32 idx = closestTriangle(triangles, points);
				vector3 normal = normalDirection(triangles[idx], points);

				const SupportPoint newPt = support(bodyA, bodyB, normal, bias);

				// If w already exists, then just stop because we can't expand any further.
				if (hasPoint(newPt.xyz, triangles, points)) {
					break;
				}

				float dist = signedDistanceToTriangle(triangles[idx], newPt.xyz, points);
				if (dist <= 0.0f) {
					break; // Can't expand
				}

				const size_t newIdx = points.size();
				points.push_back(newPt);

				// Remove triangles that face this point.
				int32 numRemoved = removeTrianglesFacingPoint(newPt.xyz, triangles, points);
				if (numRemoved == 0) {
					break;
				}

				// Find dangling edges.
				danglingEdges.clear();
				findDanglingEdges(danglingEdges, triangles);
				if (danglingEdges.size() == 0) {
					break;
				}

				// The edges should be in CCW order.
				// To create new triangles that face away from origin, we just add the new point as 'a'.
				for (size_t i = 0; i < danglingEdges.size(); ++i) {
					const Edge& edge = danglingEdges[i];

					ConvexHullTriangle triangle{ newIdx, edge.b, edge.a };

					// Make sure it's oriented properly.
					float dist = signedDistanceToTriangle(triangle, center, points);
					if (dist > 0.0f) {
						std::swap(triangle.b, triangle.c);
					}

					triangles.emplace_back(triangle);
				}
			}

			// Get the projection of the origin on the closest triangle.
			const int32 idx = closestTriangle(triangles, points);
			const ConvexHullTriangle& tri = triangles[idx];
			vector3 ptA_w = points[tri.a].xyz;
			vector3 ptB_w = points[tri.b].xyz;
			vector3 ptC_w = points[tri.c].xyz;
			vector3 lambdas = barycentricCoordinates(ptA_w, ptB_w, ptC_w, vector3(0.0f));

			// Get the point on shape A.
			vector3 ptA_a = points[tri.a].ptA;
			vector3 ptB_a = points[tri.b].ptA;
			vector3 ptC_a = points[tri.c].ptA;
			ptOnA = ptA_a * lambdas[0] + ptB_a * lambdas[1] + ptC_a * lambdas[2];

			// Get the point on shape B.
			vector3 ptA_b = points[tri.a].ptB;
			vector3 ptB_b = points[tri.b].ptB;
			vector3 ptC_b = points[tri.c].ptB;
			ptOnB = ptA_b * lambdas[0] + ptB_b * lambdas[1] + ptC_b * lambdas[2];

			// Return the penetration distance.
			vector3 delta = ptOnB - ptOnA;
			return glm::dot(delta, delta);
		}

	}
}

// GJK
namespace badger {
	namespace physics {

		bool intersectGJK(const Body* bodyA, const Body* bodyB) {
			const vector3 ORIGIN(0.0f);

			int32 numPt = 1;
			SupportPoint simplexPoints[4];
			simplexPoints[0] = support(bodyA, bodyB, vector3(1.0f, 1.0f, 1.0f), 0.0f);

			float closestDist = std::numeric_limits<float>::max();
			bool bContainsOrigin = false;
			vector3 newDir = -(simplexPoints[0].xyz);
			do {
				SupportPoint newPt = support(bodyA, bodyB, newDir, 0.0f);
				if (simplexContainsPoint(simplexPoints, newPt)) {
					break;
				}
				simplexPoints[numPt] = newPt;
				numPt++;

				// Origin cannot be in the set, therefore no collision.
				if (glm::dot(newDir, newPt.xyz - ORIGIN) < 0.0f) {
					break;
				}

				vector4 lambdas;
				bContainsOrigin = simplexSignedVolumes(simplexPoints, numPt, newDir, lambdas);
				if (bContainsOrigin) {
					break;
				}

				float dist = glm::dot(newDir, newDir);
				if (dist >= closestDist) {
					break;
				}
				closestDist = dist;

				sortValids(simplexPoints, lambdas);
				numPt = numValids(lambdas);
				bContainsOrigin = (4 == numPt);
			} while (!bContainsOrigin);

			return bContainsOrigin;
		}

		bool intersectGJK(const Body* bodyA, const Body* bodyB, float bias, vector3& ptOnA, vector3& ptOnB) {
			const vector3 ORIGIN(0.0f);

			int32 numPt = 1;
			SupportPoint simplexPoints[4];
			simplexPoints[0] = support(bodyA, bodyB, vector3(1.0f, 1.0f, 1.0f), 0.0f);

			float closestDist = std::numeric_limits<float>::max();
			bool bContainsOrigin = false;
			vector3 newDir = -(simplexPoints[0].xyz);
			do {
				SupportPoint newPt = support(bodyA, bodyB, newDir, 0.0f);
				if (simplexContainsPoint(simplexPoints, newPt)) {
					break;
				}
				simplexPoints[numPt] = newPt;
				numPt++;

				// Origin cannot be in the set, therefore no collision.
				if (glm::dot(newDir, newPt.xyz - ORIGIN) < 0.0f) {
					break;
				}

				vector4 lambdas;
				bContainsOrigin = simplexSignedVolumes(simplexPoints, numPt, newDir, lambdas);
				if (bContainsOrigin) {
					break;
				}

				float dist = glm::dot(newDir, newDir);
				if (dist >= closestDist) {
					break;
				}
				closestDist = dist;

				sortValids(simplexPoints, lambdas);
				numPt = numValids(lambdas);
				bContainsOrigin = (4 == numPt);
			} while (!bContainsOrigin);

			if (!bContainsOrigin) {
				return false;
			}

			// Check that we have a 3-simplex (EPA expects a tetrahedron).
			if (numPt == 1) {
				vector3 searchDir = -(simplexPoints[0].xyz);
				SupportPoint newPt = support(bodyA, bodyB, searchDir, 0.0f);
				simplexPoints[numPt] = newPt;
				++numPt;
			}
			if (numPt == 2) {
				vector3 ab = simplexPoints[1].xyz - simplexPoints[0].xyz;
				vector3 u, v;
				badger::calculateOrthonormalBasis(ab, u, v);

				vector3 newDir = u;
				SupportPoint newPt = support(bodyA, bodyB, newDir, 0.0f);
				simplexPoints[numPt] = newPt;
				++numPt;
			}
			if (numPt == 3) {
				vector3 ab = simplexPoints[1].xyz - simplexPoints[0].xyz;
				vector3 ac = simplexPoints[2].xyz - simplexPoints[0].xyz;
				vector3 norm = glm::cross(ab, ac);

				vector3 newDir = norm;
				SupportPoint newPt = support(bodyA, bodyB, newDir, 0.0f);
				simplexPoints[numPt] = newPt;
				++numPt;
			}

			vector3 avg(0.0f);
			for (size_t i = 0; i < 4; ++i) {
				avg += simplexPoints[i].xyz;
			}
			avg *= 0.25f;

			// Expand the simplex by the bias amount.
			for (int32 i = 0; i < numPt; ++i) {
				SupportPoint& pt = simplexPoints[i];

				vector3 dir = safeNormalize(pt.xyz - avg); // Ray from "center" to witness point
				pt.ptA += dir * bias;
				pt.ptB -= dir * bias;
				pt.xyz = pt.ptA - pt.ptB;
			}

			// Find the closest face on the CSO.
			expandingPolytopeAlgorithm(bodyA, bodyB, bias, simplexPoints, ptOnA, ptOnB);
			return true;
		}

		// Assumes no intersection.
		void closestPointGJK(const Body* bodyA, const Body* bodyB, vector3& ptOnA, vector3& ptOnB) {
			const vector3 ORIGIN(0.0f);

			float closestDist = std::numeric_limits<float>::max();
			const float bias = 0.0f;

			int32 numPt = 1;
			SupportPoint simplexPoints[4];
			simplexPoints[0] = support(bodyA, bodyB, vector3(1.0f), bias);

			vector4 lambdas(1.0f, 0.0f, 0.0f, 0.0f);
			vector3 newDir = -(simplexPoints[0].xyz);

			do {
				SupportPoint newPt = support(bodyA, bodyB, newDir, bias);

				if (simplexContainsPoint(simplexPoints, newPt)) {
					break;
				}

				simplexPoints[numPt] = newPt;
				++numPt;

				simplexSignedVolumes(simplexPoints, numPt, newDir, lambdas);
				sortValids(simplexPoints, lambdas);
				numPt = numValids(lambdas);

				float dist = glm::dot(newDir, newDir);
				if (dist >= closestDist) {
					break;
				}
				closestDist = dist;
			} while (numPt < 4);

			ptOnA = vector3(0.0f);
			ptOnB = vector3(0.0f);
			for (int32 i = 0; i < 4; ++i) {
				ptOnA += simplexPoints[i].ptA * lambdas[i];
				ptOnB += simplexPoints[i].ptB * lambdas[i];
			}
		}

		bool intersect(Body* bodyA, Body* bodyB, Contact& outContact) {
			outContact.bodyA = bodyA;
			outContact.bodyB = bodyB;
			outContact.timeOfImpact = 0.0f;

			vector3 posA = bodyA->getPosition();
			vector3 posB = bodyB->getPosition();

			if (bodyA->getShape()->getType() == Shape::EShapeType::Sphere && bodyB->getShape()->getType() == Shape::EShapeType::Sphere) {
				const ShapeSphere* sphereA = (const ShapeSphere*)bodyA->getShape();
				const ShapeSphere* sphereB = (const ShapeSphere*)bodyB->getShape();

				if (sphereSphereStatic(sphereA, sphereB, posA, posB, outContact.surfaceA_WS, outContact.surfaceB_WS)) {
					outContact.surfaceA_LS = bodyA->worldSpaceToBodySpace(outContact.surfaceA_WS);
					outContact.surfaceB_LS = bodyB->worldSpaceToBodySpace(outContact.surfaceB_WS);

					outContact.normal = safeNormalize(posA - posB);

					vector3 ab = posB - posA;
					float r = glm::length(ab) - (sphereA->getRadius() + sphereB->getRadius());

					outContact.separationDistance = r;
					return true;
				}
			} else {
				vector3 ptOnA, ptOnB;
				const float bias = 0.001f;
				if (intersectGJK(bodyA, bodyB, bias, ptOnA, ptOnB)) {
					vector3 normal = safeNormalize(ptOnB - ptOnA);
					
					ptOnA -= normal * bias;
					ptOnB += normal * bias;

					outContact.normal = normal;
					outContact.surfaceA_WS = ptOnA;
					outContact.surfaceB_WS = ptOnB;
					outContact.surfaceA_LS = bodyA->worldSpaceToBodySpace(outContact.surfaceA_WS);
					outContact.surfaceB_LS = bodyB->worldSpaceToBodySpace(outContact.surfaceB_WS);

					vector3 ab = posB - posA;
					float r = glm::length(ptOnA - ptOnB);

					outContact.separationDistance = -r;
					return true;
				}

				closestPointGJK(bodyA, bodyB, ptOnA, ptOnB);
				outContact.surfaceA_WS = ptOnA;
				outContact.surfaceB_WS = ptOnB;
				outContact.surfaceA_LS = bodyA->worldSpaceToBodySpace(outContact.surfaceA_WS);
				outContact.surfaceB_LS = bodyB->worldSpaceToBodySpace(outContact.surfaceB_WS);

				vector3 ab = posB - posA;
				float r = glm::length(ptOnA - ptOnB);
				outContact.separationDistance = r;
			}

			return false;
		}

	}
}