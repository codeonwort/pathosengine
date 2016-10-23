//#include <BossEngine/Exception.h>
#include <pathos/mesh/geometry.h>
#include <glm/glm.hpp>

#include <iostream>
#include <sstream>

namespace pathos {

	/////////////////////////////////////////////////////////////////////////////////////
	// MeshGeometry
	MeshGeometry::MeshGeometry() :drawArraysMode(false) {}
	MeshGeometry::~MeshGeometry() { this->dispose(); }

	unsigned int MeshGeometry::getIndexCount() { return indexCount; }

	void MeshGeometry::draw() {
		if (drawArraysMode){
			glDrawArrays(GL_TRIANGLES, 0, positionCount);
		}else{
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
		}
	}

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
	void MeshGeometry::updateTangentData(GLfloat* data, unsigned int length) {
		tangentData = data;
		if (!tangentBuffer){
			glGenBuffers(1, &tangentBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), tangentData, GL_STATIC_DRAW);
	}
	void MeshGeometry::updateBitangentData(GLfloat* data, unsigned int length) {
		bitangentData = data;
		if (!bitangentBuffer){
			glGenBuffers(1, &bitangentBuffer);
		}
		glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, length * sizeof(GLfloat), bitangentData, GL_STATIC_DRAW);
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

	// burrow
	void MeshGeometry::burrowVertexBuffer(const MeshGeometry* other) {
		positionData = other->positionData;
		positionCount = other->positionCount;
		positionBuffer = other->positionBuffer;
	}
	void MeshGeometry::burrowNormalBuffer(const MeshGeometry* other) {
		normalData = other->normalData;
		normalCount = other->normalCount;
		normalBuffer = other->normalBuffer;
	}
	void MeshGeometry::burrowUVBuffer(const MeshGeometry* other) {
		uvData = other->uvData;
		uvCount = other->uvCount;
		uvBuffer = other->uvBuffer;
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

	void MeshGeometry::activateTangentBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
		glVertexAttribPointer(index, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(index);
		tangentLocation = index;
	}
	void MeshGeometry::deactivateTangentBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(index);
	}

	void MeshGeometry::activateBitangentBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
		glVertexAttribPointer(index, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(index);
		bitangentLocation = index;
	}
	void MeshGeometry::deactivateBitangentBuffer(GLuint index) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(index);
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

	/*void MeshGeometry::applyTransform(glm::mat4 &transform) {
		// update positionData, normalData
		//throw Exceptions::NotImplemented(__FUNCTION__);
	}

	void MeshGeometry::scale(float value) {
		//throw Exceptions::NotImplemented(__FUNCTION__);
	}
	void MeshGeometry::scaleUV(float scaleU, float scaleV) {
		//throw Exceptions::NotImplemented(__FUNCTION__);
	}*/

	// requirements: positionData and indexData should be ready
	// it only handles indexed vertex buffer (glDrawElements).
	// vertex array should be supported.
	void MeshGeometry::calculateNormals() {
		if (drawArraysMode) calculateNormals_array();
		else calculateNormals_indexed();
	}
	void MeshGeometry::calculateNormals_array() {
		if (positionCount == 0){
			std::cerr << "Position buffer should be available to calculate normals" << std::endl;
			return;
		}
		if (normalData) delete normalData;
		normalData = new GLfloat[positionCount];
		auto P = positionData;
		for (auto i = 0; i < positionCount; i += 9){
			auto p0 = i, p1 = i + 3, p2 = i + 6;
			glm::vec3 a = glm::vec3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 b = glm::vec3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);
			auto norm = glm::normalize(glm::cross(a, b));
			normalData[i] = normalData[i + 3] = normalData[i + 6] = norm.x;
			normalData[i + 1] = normalData[i + 4] = normalData[i + 7] = norm.y;
			normalData[i + 2] = normalData[i + 5] = normalData[i + 8] = norm.z;
		}
		updateNormalData(normalData, positionCount);
	}
	void MeshGeometry::calculateNormals_indexed() {
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
			if (a == b) continue;
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
			//std::cout << accum[i].x << " " << accum[i].y << " " << accum[i].z << std::endl;
		}
		updateNormalData(normals, positionCount);
		delete accum;
		delete counts;
	}
	void MeshGeometry::calculateTangentBasis() {
		if (tangentData) delete tangentData;
		if (bitangentData) delete bitangentData;
		int numPos = positionCount / 3;
		glm::vec3* accum = new glm::vec3[numPos]; // tangent accum
		glm::vec3* accum2 = new glm::vec3[numPos]; // bitangent accum
		for (int i = 0; i < numPos; i++){
			accum[i] = glm::vec3(0.0f);
			accum2[i] = glm::vec3(0.0f);
		}

		auto P = positionData;
		auto UV = uvData;
		for (auto i = 0; i < indexCount; i += 3){
			auto i0 = indexData[i], i1 = indexData[i + 1], i2 = indexData[i + 2];
			auto p0 = i0 * 3, p1 = i1 * 3, p2 = i2 * 3;
			auto uv0 = i0 * 2, uv1 = i1 * 2, uv2 = i2 * 2;
			// delta position
			glm::vec3 dp1 = glm::vec3(P[p1] - P[p0], P[p1 + 1] - P[p0 + 1], P[p1 + 2] - P[p0 + 2]);
			glm::vec3 dp2 = glm::vec3(P[p2] - P[p0], P[p2 + 1] - P[p0 + 1], P[p2 + 2] - P[p0 + 2]);
			// delta uv
			glm::vec2 duv1 = glm::vec2(UV[uv1] - UV[uv0], UV[uv1 + 1] - UV[uv0 + 1]);
			glm::vec2 duv2 = glm::vec2(UV[uv2] - UV[uv0], UV[uv2 + 1] - UV[uv0 + 1]);
			float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);
			glm::vec3 tangent = (dp1 * duv2.y - dp2 * duv1.y) * r;
			glm::vec3 bitangent = (dp2 * duv1.x - dp1 * duv2.x) * r;
			accum[i0] += tangent; accum[i1] += tangent; accum[i2] += tangent;
			accum2[i0] += bitangent; accum2[i1] += bitangent; accum2[i2] += bitangent;
		}

		GLfloat* tangents = new GLfloat[positionCount];
		GLfloat* bitangents = new GLfloat[positionCount];
		for (auto i = 0; i < numPos; i++){
			accum[i] = glm::normalize(accum[i]);
			tangents[i * 3] = accum[i].x;
			tangents[i * 3 + 1] = accum[i].y;
			tangents[i * 3 + 2] = accum[i].z;
			accum2[i] = glm::normalize(accum2[i]);
			bitangents[i * 3] = accum2[i].x;
			bitangents[i * 3 + 1] = accum2[i].y;
			bitangents[i * 3 + 2] = accum2[i].z;
		}
		updateTangentData(tangents, positionCount);
		updateBitangentData(bitangents, positionCount);
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
		if (tangentBuffer) glDeleteBuffers(1, &tangentBuffer);
		if (bitangentBuffer) glDeleteBuffers(1, &bitangentBuffer);
		if (positionData) delete positionData;
		if (indexData) delete indexData;
		if (normalData) delete normalData;
		if (uvData) delete uvData;
		if (tangentData) delete tangentData;
		if (bitangentData) delete bitangentData;
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