#include "geometry_primitive.h"

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////
	// PlaneGeometry
	PlaneGeometry::PlaneGeometry(float width, float height, uint32 gridX, uint32 gridY, EPrimitiveInitOptions options)
		: width(width)
		, height(height)
		, gridX(gridX)
		, gridY(gridY)
	{
		buildGeometry();
		if (options & EPrimitiveInitOptions::CalculateTangentBasis) {
			calculateTangentBasis();
		}
	}

	void PlaneGeometry::buildGeometry() {
		// ccw winding
		unsigned int numPos = (gridX + 1) * (gridY + 1);
		float segW = width / gridX, segH = height / gridY;
		float x0 = -width / 2, y0 = -height / 2;

		GLfloat* positions = new GLfloat[numPos * 3];
		GLfloat* uvs = new GLfloat[numPos * 2];
		GLfloat* normals = new GLfloat[numPos * 3];
		GLuint* indices = new GLuint[gridX * gridY * 6];

		int32 k = 0;
		for (auto i = 0u; i <= gridY; i++) {
			for (auto j = 0u; j <= gridX; j++) {
				positions[k * 3] = x0 + segW * j;
				positions[k * 3 + 1] = y0 + segH * i;
				positions[k * 3 + 2] = 0.0;
				//uvs[k * 2] = (float)j / gridX;
				//uvs[k * 2 + 1] = (float)i / gridY;
				uvs[k * 2] = (float)j;
				uvs[k * 2 + 1] = (float)i;
				normals[k * 3] = normals[k * 3 + 1] = 0.0;
				normals[k * 3 + 2] = 1.0;
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

		updatePositionData(positions, numPos * 3);
		updateUVData(uvs, numPos * 2);
		updateNormalData(normals, numPos * 3);
		updateIndexData(indices, gridX * gridY * 6);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CubeGeometry
	CubeGeometry::CubeGeometry(const vector3& halfSize, EPrimitiveInitOptions options)
		: halfSize(halfSize)
	{
		buildGeometry();
		if (options & EPrimitiveInitOptions::CalculateTangentBasis) {
			calculateTangentBasis();
		}
	}

	void CubeGeometry::buildGeometry() {
		float hx = halfSize.x, hy = halfSize.y, hz = halfSize.z;
		// ccw winding
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
		GLuint indices[36] = { 0,2,1, 0,3,2, 4,1,5, 4,0,1, 4,3,0, 4,7,3, 3,6,2, 3,7,6, 7,5,6, 7,4,5, 1,6,5, 1,2,6 };

		updatePositionData(positions, 24);
		updateUVData(uvs, 16);
		updateNormalData(normals, 24);
		updateIndexData(indices, 36);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// SphereGeometry
	SphereGeometry::SphereGeometry(float radius, uint32 division, EPrimitiveInitOptions options)
		: radius(radius)
		, division(division)
	{
		buildGeometry();
		if (options & EPrimitiveInitOptions::CalculateTangentBasis) {
			calculateTangentBasis();
		}
	}

	void SphereGeometry::buildGeometry() {
		uint32 numVertices = division * division;
		uint32 numTriangles = division * (division - 1) * 2;

		// ccw winding
		GLfloat* positions = new GLfloat[numVertices * 3];
		GLfloat* uvs = new GLfloat[numVertices * 2];
		GLfloat* normals = new GLfloat[numVertices * 3];
		GLuint* indices = new GLuint[numTriangles * 3];

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

				positions[3 * k + 0] = radius * x;
				positions[3 * k + 1] = radius * y;
				positions[3 * k + 2] = radius * z;

				normals[3 * k + 0] = x;
				normals[3 * k + 1] = y;
				normals[3 * k + 2] = z;

				uvs[2 * k] = theta / pi2;
				uvs[2 * k + 1] = (phi + pi_2) / pi;
				if (i != division - 1) {
					if (j != division - 1) {
						indices[6 * k + 0] = k;
						indices[6 * k + 1] = k + division;
						indices[6 * k + 2] = k + division + 1;
						indices[6 * k + 3] = k;
						indices[6 * k + 4] = k + division + 1;
						indices[6 * k + 5] = k + 1;
					} else {
						indices[6 * k + 0] = k;
						indices[6 * k + 1] = k + division;
						indices[6 * k + 2] = k + 1;
						indices[6 * k + 3] = k;
						indices[6 * k + 4] = k + 1;
						indices[6 * k + 5] = k + 1 - division;
					}
				}
				k++;
			}
		}

		updatePositionData(positions, numVertices * 3);
		updateUVData(uvs, numVertices * 2);
		updateNormalData(normals, numVertices * 3);
		updateIndexData(indices, numTriangles * 3);
	}

}
