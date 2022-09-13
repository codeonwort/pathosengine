#pragma once

#include "badger/types/vector_types.h"

struct AABB
{
	static AABB fromMinMax(const vector3& minV, const vector3& maxV) {
		return AABB{ minV, maxV };
	}
	static AABB fromCenterAndHalfSize(const vector3& center, const vector3& halfSize) {
		return AABB{ center - halfSize, center + halfSize };
	}

	inline vector3 getCenter() const {
		return 0.5f * (minBounds + maxBounds);
	}
	inline vector3 getHalfSize() const {
		return 0.5f * (maxBounds - minBounds);
	}
	inline vector3 getExtent() const {
		return (maxBounds - minBounds);
	}

	vector3 minBounds;
	vector3 maxBounds;
};

inline AABB operator+(const AABB& a, const AABB& b)
{
	vector3 minBounds = (glm::min)(a.minBounds, b.minBounds);
	vector3 maxBounds = (glm::max)(a.maxBounds, b.maxBounds);
	return AABB::fromMinMax(minBounds, maxBounds);
}
