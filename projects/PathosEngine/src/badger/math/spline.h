#pragma once

#include "badger/types/vector_types.h"
#include "badger/types/int_types.h"
#include <vector>

class HermiteSpline
{
public:
	void clearPoints();

	// You must call updateSpline() after all points are added.
	void addPoint(const vector3& point, const vector3& tangent);

	void updateSpline();

	float getTotalTime() const;
	float getTotalDistance() const;

	vector3 locationAtTime(float time);
	vector3 locationAtDistance(float distance);

private:
	// Function value of P_i(t) where P_i is the hermite spline
	// defined by points[i], points[i+1], tangents[i], and tangents[i+1]
	vector3 evaluate(uint32 i, float t);

	// Arc-length from P_i(0) to P_i(t), where P_i is the hermite spline
	// defined by points[i], points[i+1], tangents[i], and tangents[i+1]
	float getArcLength(uint32 i, float t) const;

	bool invalidated = true;

	std::vector<vector3> points;
	std::vector<vector3> tangents;
	float totalDistance = 0.0f;

	// xyz = location, w = distance
	std::vector<vector4> reparam;
};
