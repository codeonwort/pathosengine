#include "geometry_primitive.h"

// NOTE: CCW winding

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////
	// PlaneGeometry
	PlaneGeometry::PlaneGeometry(
		float inWidth, float inHeight,
		uint32 inGridX, uint32 inGridY,
		EPrimitiveInitOptions options)
		: width(inWidth)
		, height(inHeight)
		, gridX(inGridX)
		, gridY(inGridY)
	{
		buildGeometry();
		if (options & EPrimitiveInitOptions::CalculateTangentBasis) {
			calculateTangentBasis();
		}
	}

	void PlaneGeometry::buildGeometry() {
		const uint32 numVertices = (gridX + 1) * (gridY + 1);
		const float segW = width / gridX, segH = height / gridY;
		const float x0 = -width / 2, y0 = -height / 2;

		std::vector<GLfloat> positions(numVertices * 3);
		std::vector<GLfloat> uvs(numVertices * 2);
		std::vector<GLfloat> normals(numVertices * 3);
		std::vector<GLuint> indices(gridX * gridY * 6);

		size_t k = 0;
		for (auto i = 0u; i <= gridY; i++) {
			for (auto j = 0u; j <= gridX; j++) {
				positions[size_t(k * 3)] = x0 + segW * j;
				positions[size_t(k * 3 + 1)] = y0 + segH * i;
				positions[size_t(k * 3 + 2)] = 0.0f;
				//uvs[k * 2] = (float)j / gridX;
				//uvs[k * 2 + 1] = (float)i / gridY;
				uvs[size_t(k * 2)] = (float)j;
				uvs[size_t(k * 2 + 1)] = (float)i;
				normals[size_t(k * 3)] = 0.0f;
				normals[size_t(k * 3 + 1)] = 0.0f;
				normals[size_t(k * 3 + 2)] = 1.0f;
				k++;
			}
		}

		k = 0;
		for (auto i = 0u; i < gridY; i++) {
			auto baseY = i * (gridX + 1);
			for (auto j = 0u; j < gridX; j++) {
				indices[k] = baseY + j;
				indices[k + 1] = baseY + j + 1;
				indices[k + 2] = baseY + j + (gridX + 1);
				indices[k + 3] = baseY + j + 1;
				indices[k + 4] = baseY + j + (gridX + 1) + 1;
				indices[k + 5] = baseY + j + (gridX + 1);
				k += 6;
			}
		}

		updatePositionData(positions.data(), (uint32)positions.size());
		updateUVData(uvs.data(), (uint32)uvs.size());
		updateNormalData(normals.data(), (uint32)normals.size());
		updateIndexData(indices.data(), (uint32)indices.size());
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CubeGeometry
	CubeGeometry::CubeGeometry(
		const vector3& inHalfSize,
		bool smoothing,
		EPrimitiveInitOptions options)
		: halfSize(inHalfSize)
	{
		buildGeometry(smoothing);
		if (options & EPrimitiveInitOptions::CalculateTangentBasis) {
			calculateTangentBasis();
		}
	}

	void CubeGeometry::buildGeometry(bool smoothing) {
		if (smoothing) {
			buildGeometry_smoothing();
		} else {
			buildGeometry_noSmoothing();
		}
	}

	void CubeGeometry::buildGeometry_smoothing() {
		float hx = halfSize.x, hy = halfSize.y, hz = halfSize.z;
		GLfloat positions[24] = {
			hx, hy, hz,
			hx, -hy, hz,
			-hx, -hy, hz,
			-hx, hy, hz,
			hx, hy, -hz,
			hx, -hy, -hz,
			-hx, -hy, -hz,
			-hx, hy, -hz
		};
		GLfloat uvs[16] = { 1,1, 1,0, 0,0, 0,1, 1,1, 1,0, 0,0, 0,1 };
		GLfloat normals[24] = {
			1, 1, 1,
			1, -1, 1,
			-1, -1, 1,
			-1, 1, 1,
			1, 1, -1,
			1, -1, -1,
			-1, -1, -1,
			-1, 1, -1 };
		GLuint indices[36] = {
			0,2,1, 0,3,2,
			4,1,5, 4,0,1,
			4,3,0, 4,7,3,
			3,6,2, 3,7,6,
			7,5,6, 7,4,5,
			1,6,5, 1,2,6 };

		updatePositionData(positions, 24);
		updateUVData(uvs, 16);
		updateNormalData(normals, 24);
		updateIndexData(indices, 36);
	}

	void CubeGeometry::buildGeometry_noSmoothing() {
		float hx = halfSize.x, hy = halfSize.y, hz = halfSize.z;
		
		vector3 corners[8] = {
			{hx, hy, hz},
			{hx, -hy, hz},
			{-hx, -hy, hz},
			{-hx, hy, hz},
			{hx, hy, -hz},
			{hx, -hy, -hz},
			{-hx, -hy, -hz},
			{-hx, hy, -hz}
		};
		GLuint sourceIndices[36] = {
			0,2,1, 0,3,2,
			4,1,5, 4,0,1,
			4,3,0, 4,7,3,
			3,6,2, 3,7,6,
			7,5,6, 7,4,5,
			1,6,5, 1,2,6 };

		std::vector<GLfloat> positions;
		std::vector<GLfloat> uvs;
		std::vector<GLfloat> normals;
		std::vector<GLuint> indices;

#define PUSH_VEC3(vs, v) { vs.push_back(v.x); vs.push_back(v.y); vs.push_back(v.z); }

		// #todo-geometry: Unnecessary runtime generation. Use literals.
		int32 k = 0;
		for (size_t i = 0; i < 36; i += 3)
		{
			GLuint j0 = sourceIndices[i + 0];
			GLuint j1 = sourceIndices[i + 1];
			GLuint j2 = sourceIndices[i + 2];
			vector3 P0 = corners[j0];
			vector3 P1 = corners[j1];
			vector3 P2 = corners[j2];
			vector3 N = glm::normalize(glm::cross(P1 - P0, P2 - P0));

			PUSH_VEC3(positions, P0);
			PUSH_VEC3(positions, P1);
			PUSH_VEC3(positions, P2);
			if (i % 6 == 0) {
				uvs.push_back(1.0f); uvs.push_back(1.0f);
				uvs.push_back(0.0f); uvs.push_back(0.0f);
				uvs.push_back(1.0f); uvs.push_back(0.0f);
			} else {
				uvs.push_back(1.0f); uvs.push_back(1.0f);
				uvs.push_back(0.0f); uvs.push_back(1.0f);
				uvs.push_back(0.0f); uvs.push_back(0.0f);
			}
			PUSH_VEC3(normals, N);
			PUSH_VEC3(normals, N);
			PUSH_VEC3(normals, N);
			indices.push_back(k);
			indices.push_back(k + 1);
			indices.push_back(k + 2);
			k += 3;
		}

#undef PUSH_VEC3

		updatePositionData(positions.data(), (uint32)positions.size());
		updateUVData(uvs.data(), (uint32)uvs.size());
		updateNormalData(normals.data(), (uint32)normals.size());
		updateIndexData(indices.data(), (uint32)indices.size());
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// SphereGeometry
	SphereGeometry::SphereGeometry(
		float inRadius,
		uint32 inDivision,
		EPrimitiveInitOptions options)
		: radius(inRadius)
		, division(inDivision)
	{
		buildGeometry();
		if (options & EPrimitiveInitOptions::CalculateTangentBasis) {
			calculateTangentBasis();
		}
	}

	void SphereGeometry::buildGeometry() {
		uint32 numVertices = division * division;
		uint32 numTriangles = division * (division - 1) * 2;

		std::vector<GLfloat> positions(numVertices * 3);
		std::vector<GLfloat> uvs(numVertices * 2);
		std::vector<GLfloat> normals(numVertices * 3);
		std::vector<GLuint> indices(numTriangles * 3);

		float pi = atan(1.f) * 4.f;
		float pi_2 = pi * .5f, pi2 = pi + pi;
		float theta, phi;
		float x, y, z;
		int k = 0;

		for (size_t i = 0; i < division; i++) {
			phi = pi_2 - pi * (float)(i) / (division - 1);
			for (size_t j = 0; j < division; j++) {
				theta = pi2 * (float)j / (division - 1);
				x = cos(phi) * cos(theta);
				y = cos(phi) * sin(theta);
				z = sin(phi);

				positions[size_t(3 * k + 0)] = radius * x;
				positions[size_t(3 * k + 1)] = radius * y;
				positions[size_t(3 * k + 2)] = radius * z;

				normals[size_t(3 * k + 0)] = x;
				normals[size_t(3 * k + 1)] = y;
				normals[size_t(3 * k + 2)] = z;

				uvs[size_t(2 * k + 0)] = theta / pi2;
				uvs[size_t(2 * k + 1)] = (phi + pi_2) / pi;
				if (i != division - 1) {
					if (j != division - 1) {
						indices[size_t(6 * k + 0)] = k;
						indices[size_t(6 * k + 1)] = k + division;
						indices[size_t(6 * k + 2)] = k + division + 1;
						indices[size_t(6 * k + 3)] = k;
						indices[size_t(6 * k + 4)] = k + division + 1;
						indices[size_t(6 * k + 5)] = k + 1;
					} else {
						indices[size_t(6 * k + 0)] = k;
						indices[size_t(6 * k + 1)] = k + division;
						indices[size_t(6 * k + 2)] = k + 1;
						indices[size_t(6 * k + 3)] = k;
						indices[size_t(6 * k + 4)] = k + 1;
						indices[size_t(6 * k + 5)] = k + 1 - division;
					}
				}
				k++;
			}
		}

		updatePositionData(positions.data(), (uint32)positions.size());
		updateUVData(uvs.data(), (uint32)uvs.size());
		updateNormalData(normals.data(), (uint32)normals.size());
		updateIndexData(indices.data(), (uint32)indices.size());
	}

}
