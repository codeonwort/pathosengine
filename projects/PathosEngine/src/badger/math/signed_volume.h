#pragma once

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"

#include <limits>

namespace badger {

	// s1, s2: Two points on the line segment.
	// p: The point to project onto the line.
	vector3 projectPointOntoLine(const vector3& s1, const vector3& s2, const vector3& p);

	// s1, s2: End points of line segement.
	// 
	// Returns barycentric coordinates of the origin projected onto line segment [s1, s2].
	// Denote that projected point as p, the return value as (k1, k2), then p = k1*s1 + k2*s2.
	vector2 signedVolume1D(const vector3& s1, const vector3& s2);

	// s1, s2, s3: Vertices of triangle.
	// Returns barycentric coordinates of the origin projected onto triangle (s1, s2, s3).
	// Denote that projected point as p, the return value as (k1, k2, k3), then p = k1*s1 + k2*s2 + k3*s3.
	vector3 signedVolume2D(const vector3& s1, const vector3& s2, const vector3& s3);

	// s1, s2, s3, s4: Vertices of tetrahedron.
	// Returns barycentric coordinates of the origin projected onto the tetrahedron.
	// Denote that projected point as p, the return value as (k1, k2, k3, k4), then p = k1*s1 + k2*s2 + k3*s3 + k4*s4.
	vector4 signedVolume3D(const vector3& s1, const vector3& s2, const vector3& s3, const vector3& s4);

}
