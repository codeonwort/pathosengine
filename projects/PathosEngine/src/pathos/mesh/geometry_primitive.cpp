#include "geometry_primitive.h"

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////
	// PlaneGeometry
	PlaneGeometry::PlaneGeometry(float width, float height, unsigned int gridX, unsigned int gridY) : width(width), height(height) {
		this->gridX = gridX;
		this->gridY = gridY;
		buildGeometry();
	}

	void PlaneGeometry::buildGeometry() {
		// ccw winding
		unsigned int numPos = (gridX + 1) * (gridY + 1);
		float segW = width / gridX, segH = height / gridY;
		float x0 = -width / 2, y0 = -height / 2;

		GLfloat* positionData = new GLfloat[numPos * 3];
		GLfloat* uvData = new GLfloat[numPos * 2];
		GLfloat* normalData = new GLfloat[numPos * 3];
		GLuint* indexData = new GLuint[gridX * gridY * 6];

		int k = 0;
		for (auto i = 0; i <= gridY; i++) {
			for (auto j = 0; j <= gridX; j++) {
				positionData[k * 3] = x0 + segW * j;
				positionData[k * 3 + 1] = y0 + segH * i;
				positionData[k * 3 + 2] = 0.0;
				uvData[k * 2] = (float)j / gridX;
				uvData[k * 2 + 1] = (float)i / gridY;
				normalData[k * 3] = normalData[k * 3 + 1] = 0.0;
				normalData[k * 3 + 2] = 1.0;
				k++;
			}
		}

		k = 0;
		for (auto i = 0; i < gridY; i++) {
			auto baseY = i * (gridX + 1);
			for (auto j = 0; j < gridX; j++) {
				indexData[k] = baseY + j;
				indexData[k + 1] = baseY + j + 1;
				indexData[k + 2] = baseY + j + (gridX + 1);
				indexData[k + 3] = baseY + j + 1;
				indexData[k + 4] = baseY + j + (gridX + 1) + 1;
				indexData[k + 5] = baseY + j + (gridX + 1);
				k += 6;
			}
		}

		updateVertexData(positionData, numPos * 3);
		updateUVData(uvData, numPos * 2);
		updateNormalData(normalData, numPos * 3);
		updateIndexData(indexData, gridX * gridY * 6);

		delete[] positionData;
		delete[] uvData;
		delete[] normalData;
		delete[] indexData;

		/*float hw = width / 2, hh = height / 2;
		GLfloat* vertexData = new GLfloat[12]{ -hw,-hh,0, hw,-hh,0, hw,hh,0, -hw,hh,0 };
		GLfloat* uvData = new GLfloat[8]{ 0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f, 0.0f,1.0f };
		GLfloat* normalData = new GLfloat[12]{ 0.0f,0,1, 0,0,1, 0,0,1, 0,0,1 };
		GLuint* indexData = new GLuint[6]{ 0,1,2, 0,2,3 };
		updateVertexData(vertexData, 12);
		updateUVData(uvData, 8);
		updateNormalData(normalData, 12);
		updateIndexData(indexData, 6);*/
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// CubeGeometry
	CubeGeometry::CubeGeometry(const glm::vec3& halfSize) :halfSize(halfSize) {
		buildGeometry();
	}
	void CubeGeometry::buildGeometry() {
		float hx = halfSize.x, hy = halfSize.y, hz = halfSize.z;
		// ccw winding
		GLfloat* vertexData = new GLfloat[24]{
			hx, hy, hz,
			hx, -hy, hz,
			-hx, -hy, hz,
			-hx, hy, hz,
			hx, hy, -hz,
			hx, -hy, -hz,
			-hx, -hy, -hz,
			-hx, hy, -hz
		};
		GLfloat* uvData = new GLfloat[16]{ 1,1, 1,0, 0,0, 0,1, 1,1, 1,0, 0,0, 0,1 };
		GLfloat* normalData = new GLfloat[24]{
			1, 1, 1,
			1, -1, 1,
			-1, -1, 1,
			-1, 1, 1,
			1, 1, -1,
			1, -1, -1,
			-1, -1, -1,
			-1, 1, -1 };
		GLuint* indexData = new GLuint[36]{ 0,2,1, 0,3,2, 4,1,5, 4,0,1, 4,3,0, 4,7,3, 3,6,2, 3,7,6, 7,5,6, 7,4,5, 1,6,5, 1,2,6 };

		updateVertexData(vertexData, 24);
		updateUVData(uvData, 16);
		updateNormalData(normalData, 24);
		updateIndexData(indexData, 36);

		delete[] vertexData;
		delete[] uvData;
		delete[] normalData;
		delete[] indexData;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// SphereGeometry
	SphereGeometry::SphereGeometry(float radius, unsigned int division) :radius(radius), division(division) {
		buildGeometry();
	}
	void SphereGeometry::buildGeometry() {
		unsigned int numVertices = division*division;
		unsigned int numTriangles = division*(division - 1) * 2;
		// ccw winding
		GLfloat* vertexData = new GLfloat[numVertices * 3];
		GLfloat* uvData = new GLfloat[numVertices * 2];
		GLfloat* normalData = new GLfloat[numVertices * 3];
		GLuint* indexData = new GLuint[numTriangles * 3];

		float pi = atan(1.f) * 4.f;
		float pi_2 = pi * .5f, pi2 = pi + pi;
		float theta, phi;
		float x, y, z;
		int k = 0;
		for (size_t i = 0; i < division; i++) {
			phi = pi_2 - pi * (float)(i) / (division - 1);
			for (size_t j = 0; j < division; j++) {
				theta = pi2 * (float)j / (division - 1);
				x = radius * cos(phi) * cos(theta);
				y = radius * cos(phi) * sin(theta);
				z = radius * sin(phi);
				vertexData[3 * k] = normalData[3 * k] = x;
				vertexData[3 * k + 1] = normalData[3 * k + 1] = y;
				vertexData[3 * k + 2] = normalData[3 * k + 2] = z;
				uvData[2 * k] = theta;
				uvData[2 * k + 1] = phi;
				if (i != division - 1) {
					if (j != division - 1) {
						indexData[6 * k] = k;
						indexData[6 * k + 1] = k + division;
						indexData[6 * k + 2] = k + division + 1;
						indexData[6 * k + 3] = k;
						indexData[6 * k + 4] = k + division + 1;
						indexData[6 * k + 5] = k + 1;
					}
					else {
						indexData[6 * k] = k;
						indexData[6 * k + 1] = k + division;
						indexData[6 * k + 2] = k + 1;
						indexData[6 * k + 3] = k;
						indexData[6 * k + 4] = k + 1;
						indexData[6 * k + 5] = k + 1 - division;
					}
				}
				k++;
			}
		}

		updateVertexData(vertexData, numVertices * 3);
		updateUVData(uvData, numVertices * 2);
		updateNormalData(normalData, numVertices * 3);
		updateIndexData(indexData, numTriangles * 3);

		delete[] vertexData;
		delete[] uvData;
		delete[] normalData;
		delete[] indexData;
	}

}