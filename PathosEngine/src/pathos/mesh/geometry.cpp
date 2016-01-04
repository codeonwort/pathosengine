//#include <BossEngine/Exception.h>
#include <pathos/mesh/geometry.h>
#include <glm/glm.hpp>

#include <iostream>
#include <sstream>

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////
	// MeshGeometry
	MeshGeometry::MeshGeometry() {
		positionData = nullptr;
		indexData = nullptr;
		normalData = nullptr;
		positionBuffer = uvBuffer = normalBuffer = indexBuffer = 0;
		vertexLocation = uvLocation = normalLocation = 0;
		vertexActivated = uvActivated = normalActivated = false;
	}

	MeshGeometry::~MeshGeometry() {
		this->dispose();
	}

	unsigned int MeshGeometry::getIndexCount() { return indexCount; }

	void MeshGeometry::updateVertexData(GLfloat* data, unsigned int length) {
		positionData = data;
		positionCount = length;
		if (!positionBuffer) {
			glGenBuffers(1, &positionBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), positionData, GL_STATIC_DRAW);
	}
	void MeshGeometry::updateUVData(GLfloat* data, unsigned int length) {
		uvData = data;
		uvCount = length;
		if (!uvBuffer) {
			glGenBuffers(1, &uvBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), uvData, GL_STATIC_DRAW);
	}
	void MeshGeometry::updateNormalData(GLfloat* data, unsigned int length) {
		normalData = data;
		normalCount = length;
		if (!normalBuffer) {
			glGenBuffers(1, &normalBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), normalData, GL_STATIC_DRAW);
	}
	void MeshGeometry::updateIndexData(GLuint* data, unsigned int length) {
		indexData = data;
		indexCount = length;
		if (!indexBuffer) {
			glGenBuffers(1, &indexBuffer);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, length * sizeof(GLuint), indexData, GL_STATIC_DRAW);
	}

	void MeshGeometry::activateVertexBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(index);
		vertexActivated = true;
		vertexLocation = index;
	}
	void MeshGeometry::deactivateVertexBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(index);
		vertexActivated = false;
	}

	void MeshGeometry::activateUVBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(index);
		uvActivated = true;
		uvLocation = index;
	}
	void MeshGeometry::deactivateUVBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(index);
		uvActivated = false;
	}

	void MeshGeometry::activateNormalBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glVertexAttribPointer(index, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(index);
		normalActivated = true;
		normalLocation = index;
	}
	void MeshGeometry::deactivateNormalBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(index);
		normalActivated = false;
	}

	void MeshGeometry::activateIndexBuffer() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	}
	void MeshGeometry::deactivateIndexBuffer() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	GLuint MeshGeometry::getVertexLocation() { return vertexLocation; }
	GLuint MeshGeometry::getUVLocation() { return uvLocation; }
	GLuint MeshGeometry::getNormalLocation() { return normalLocation; }
	bool MeshGeometry::isVertexActivated() { return vertexActivated; }
	bool MeshGeometry::isUVActivated() { return uvActivated; }
	bool MeshGeometry::isNormalActivated() { return normalActivated; }

	void MeshGeometry::applyTransform(glm::mat4 &transform) {
		// update positionData, normalData
	}

	void MeshGeometry::scale(float value) {
		//throw Exceptions::NotImplemented(__FUNCTION__);
	}
	void MeshGeometry::scaleUV(float scaleU, float scaleV) {
		//throw Exceptions::NotImplemented(__FUNCTION__);
	}

	void MeshGeometry::calculateNormals() {
		if (normalData) delete normalData;
		int numPos = positionCount / 3;
		glm::vec3* accum = new glm::vec3[numPos];
		unsigned int* counts = new unsigned int[numPos];
		for (int i = 0; i < numPos; i++){
			accum[i] = glm::vec3(0.0f);
			counts[i] = 0;
		}

		auto P = positionData;
		for (auto i = 0; i < indexCount; i += 3){
			auto i0 = indexData[i], i1 = indexData[i + 1], i2 = indexData[i + 2];
			auto p0 = i0 * 3, p1 = i1 * 3, p2 = i2 * 3;
			glm::vec3 a = glm::vec3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 b = glm::vec3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);
			auto norm = glm::normalize(glm::cross(a, b));

			accum[i0] *= counts[i0]; accum[i1] *= counts[i1]; accum[i2] *= counts[i2];
			accum[i0] += norm; accum[i1] += norm; accum[i2] += norm;
			counts[i0] ++; counts[i1] ++; counts[i2] ++;
			accum[i0] /= counts[i0]; accum[i1] /= counts[i1]; accum[i2] /= counts[i2];
		}

		GLfloat* normals = new GLfloat[positionCount];
		for (auto i = 0; i < numPos; i++){
			accum[i] = glm::normalize(accum[i]);
			normals[i * 3] = accum[i].x;
			normals[i * 3 + 1] = accum[i].y;
			normals[i * 3 + 2] = accum[i].z;
		}
		updateNormalData(normals, positionCount);
		delete accum;
		delete counts;
	}

	void MeshGeometry::uploadPosition() {
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, positionCount * sizeof(GLfloat), positionData, GL_STATIC_DRAW);
	}
	void MeshGeometry::dispose() {
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &indexBuffer);
		glDeleteBuffers(1, &uvBuffer);
		glDeleteBuffers(1, &normalBuffer);
		if (positionData) delete positionData;
		if (indexData) delete indexData;
		if (normalData) delete normalData;
		if (uvData) delete uvData;
	}
	
	/////////////////////////////////////////////////////////////////////////////////////
	// PlaneGeometry
	PlaneGeometry::PlaneGeometry(float width, float height, unsigned int gridX, unsigned int gridY): width(width), height(height) {
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
		for (auto i = 0; i <= gridY; i++){
			for (auto j = 0; j <= gridX; j++){
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
		for (auto i = 0; i < gridY; i++){
			auto baseY = i * (gridX + 1);
			for (auto j = 0; j < gridX; j++){
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
				theta = pi2 * (float)j / division;
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
	}

}