#include "collision.h"
#include "shape.h"
#include "badger/math/signed_volume.h"
#include "badger/math/convex_hull.h"

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

	}
}

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
			return 0.0f;
		}

		static int32 closestTriangle(const std::vector<ConvexHullTriangle>& triangles, const std::vector<SupportPoint>& points) {
			return 0;
		}

		static bool hasPoint(const vector3& w, const std::vector<ConvexHullTriangle>& triangles, const std::vector<SupportPoint>& points) {
			return false;
		}

		static int32 removeTrianglesFacingPoint(const vector3& pt, std::vector<ConvexHullTriangle>& triangles, const std::vector<SupportPoint>& points) {
			return 0;
		}

		static void findDanglingEdges(std::vector<Edge>& danglingEdges, const std::vector<ConvexHullTriangle>& triangles) {
			//
		}

		// #todo-physics: Stub code for EPA
		static float expandingPolytopeAlgorithm(
			const Body* bodyA, const Body* bodyB,
			float bias, const SupportPoint simplexPoints[4],
			vector3& ptOnA, vector3& ptOnB)
		{
			return 0.0f;
		}

	}
}
