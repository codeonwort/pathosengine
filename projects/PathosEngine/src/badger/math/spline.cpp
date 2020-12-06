#include "spline.h"
#include "badger/assertion/assertion.h"

#define HERMITE_SPLINE_SEGMENTATION     10
#define INV_HERMITE_SPLINE_SEGMENTATION (1.0f / (float)HERMITE_SPLINE_SEGMENTATION)

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

	int32 i = findReparamIndex(distance);
	if (i >= 0) {
		// Distance-time relation is not linear, but do it anyway.
		float progress = (distance - reparam[i].w) / (reparam[i + 1].w - reparam[i].w);
		return vector3(reparam[i]) * (1.0f - progress) + vector3(reparam[i + 1]) * progress;
	}

	CHECK_NO_ENTRY();
	return vector3(0.0f);
}

vector3 HermiteSpline::tangentAtTime(float time)
{
	if (time < 0.0f || points.size() < 2) return tangents[0];
	else if (time >= getTotalTime()) return tangents[tangents.size() - 1];

	return derivative((uint32)time, time - (uint32)time);
}

vector3 HermiteSpline::tangentAtDistance(float distance)
{
	CHECK(!invalidated);

	if (reparam.size() == 0) return vector3(0.0f);
	if (distance <= 0.0f) return tangents[0];
	else if (distance >= totalDistance) return tangents[tangents.size() - 1];

	int32 reparamIndex = findReparamIndex(distance);
	if (reparamIndex >= 0) {
		int32 i = int32(reparamIndex * INV_HERMITE_SPLINE_SEGMENTATION);
		float progress = (float)(reparamIndex - i * HERMITE_SPLINE_SEGMENTATION) * INV_HERMITE_SPLINE_SEGMENTATION;
		// #todo-spline: slerp between nearest tangents?
		progress += INV_HERMITE_SPLINE_SEGMENTATION * (distance - reparam[reparamIndex].w) / (reparam[reparamIndex + 1].w - reparam[reparamIndex].w);
		return derivative(i, progress);
	}

	CHECK_NO_ENTRY();
	return vector3(0.0f);
}

vector3 HermiteSpline::evaluate(uint32 i, float t) const
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

vector3 HermiteSpline::derivative(uint32 i, float t) const
{
	float tt = t * t;
	vector3 p0 = points[i], p1 = points[i + 1];
	vector3 m0 = tangents[i], m1 = tangents[i + 1];

	return (6.0f * tt - 6.0f * t) * p0
		+ (3.0f * tt - 4.0f * t + 1.0f) * m0
		+ (-6.0f * tt + 6.0f * t) * p1
		+ (3.0f * tt - 2.0f * t) * m1;
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
	for (uint32 j = 0; j < 5; ++j) {
		float x = 0.5f * t * (abscissas[j] + 1.0f);
		// Derivative of a hermite spline is not a polynomial, but do it anyway.
		vector3 v = derivative(i, x);
		L += weights[j] * glm::length(v);
	}
	L *= 0.5f * t;

	return L;
}

int32 HermiteSpline::findReparamIndex(float distance) const
{
	// #todo-spline: Binary search
	uint32 numReparam = (uint32)reparam.size();
	for (uint32 i = 0; i < numReparam - 1; ++i) {
		if (reparam[i].w <= distance && distance < reparam[i + 1].w) {
			return i;
		}
	}
	return -1;
}
