#include "spline.h"
#include "badger/assertion/assertion.h"

#define HERMITE_SPLINE_SEGMENTATION   10

void HermiteSpline::clearPoints()
{
	points.clear();
	tangents.clear();
}

void HermiteSpline::addPoint(const vector3& point, const vector3& tangent)
{
	points.push_back(point);
	tangents.push_back(tangent);

	invalidated = true;
}

void HermiteSpline::updateSpline()
{
	uint32 numPoints = (uint32)points.size();
	totalDistance = 0.0f;

	reparam.clear();
	reparam.reserve((numPoints - 1) * HERMITE_SPLINE_SEGMENTATION);

	for (uint32 i = 0; i < numPoints - 1; ++i) {
		for (uint32 j = 0; j < HERMITE_SPLINE_SEGMENTATION; ++j) {
			float progress = (float)j / (float)HERMITE_SPLINE_SEGMENTATION;
			vector3 location = evaluate(i, progress);
			float arcLength = getArcLength(i, progress);
			reparam.push_back(vector4(location, totalDistance + arcLength));
		}
		totalDistance += getArcLength(i, 1.0f);
	}
	reparam.push_back(vector4(points[points.size() - 1], totalDistance));

	invalidated = false;
}

float HermiteSpline::getTotalTime() const
{
	return points.size() < 2 ? 0.0f : (float)points.size() - 1.0f;
}

float HermiteSpline::getTotalDistance() const
{
	CHECK(!invalidated);

	return totalDistance;
}

vector3 HermiteSpline::locationAtTime(float time)
{
	if (time < 0.0f || points.size() < 2) return points[0];
	else if (time >= getTotalTime()) return points[points.size() - 1];

	return evaluate((uint32)time, time - (uint32)time);
}

vector3 HermiteSpline::locationAtDistance(float distance)
{
	CHECK(!invalidated);

	if (reparam.size() == 0) return vector3(0.0f);
	if (distance <= 0.0f) return points[0];
	else if (distance >= totalDistance) return points[points.size() - 1];

	uint32 numReparam = (uint32)reparam.size();
	for (uint32 i = 0; i < numReparam - 1; ++i) {
		// #todo-spline: Binary search
		if (reparam[i].w <= distance && distance < reparam[i + 1].w) {
			// Of course distance-time relation is not linear, but do it anyway.
			float progress = (distance - reparam[i].w) / (reparam[i + 1].w - reparam[i].w);
			return vector3(reparam[i]) * (1.0f - progress) + vector3(reparam[i + 1]) * progress;
		}
	}

	CHECK_NO_ENTRY();
	return vector3(0.0f);
}

vector3 HermiteSpline::evaluate(uint32 i, float t)
{
	vector3 p0 = points[i], p1 = points[i + 1];
	vector3 m0 = tangents[i], m1 = tangents[i + 1];
	float tt = t * t;
	float ttt = tt * t;

	return (2.0f * ttt - 3.0f * tt + 1.0f) * p0
		+ (ttt - 2.0f * tt + t) * m0
		+ (-2.0f * ttt + 3.0f * tt) * p1
		+ (ttt - tt) * m1;
}

float HermiteSpline::getArcLength(uint32 i, float t) const
{
	// Gauss-Legendre Quadrature
	static const float abscissas[] = {
		0.0f,
		-0.5384693f,
		0.5384693f,
		-0.90617985f,
		0.90617985f
	};
	static const float weights[] = {
		0.5688889f,
		0.47862867f,
		0.47862867f,
		0.23692688f,
		0.23692688f
	};

	vector3 p0 = points[i], p1 = points[i + 1];
	vector3 m0 = tangents[i], m1 = tangents[i + 1];

	float L = 0.0f;
	for (uint32 i = 0; i < 5; ++i) {
		float x = 0.5f * t * (abscissas[i] + 1.0f);
		float xx = x * x;
		float xxx = xx * x;
		// Derivative of a hermite spline is not a polynomial, but do it anyway.
		vector3 v = (6.0f * xx - 6.0f * x) * p0
			+ (3.0f * xx - 4.0f * x + 1.0f) * m0
			+ (-6.0f * xx + 6.0f * x)* p1
			+ (3.0f * xx - 2.0f * x) * m1;
		L += weights[i] * glm::length(v);
	}
	L *= 0.5f * t;

	return L;
}
