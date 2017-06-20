#pragma once

#include "glm/matrix.hpp"
#include "pathos/core.h"
#include <GL/glew.h>
#include <vector>

namespace pathos {

	class MeshGeometry : public NamedObject {
	private:
		GLfloat *positionData = nullptr, *uvData = nullptr, *normalData = nullptr, *tangentData = nullptr, *bitangentData = nullptr;
		GLuint *indexData = nullptr;
		GLuint positionBuffer = 0, uvBuffer = 0, normalBuffer = 0, indexBuffer = 0, tangentBuffer = 0, bitangentBuffer = 0;
		GLuint vertexLocation, uvLocation, normalLocation, tangentLocation, bitangentLocation; // xxLocation is valid only if xxActivated == true
		bool vertexActivated = false, uvActivated = false, normalActivated = false;
		unsigned int positionCount = 0, uvCount = 0, normalCount = 0, indexCount = 0; // it's array length, not actual count!

		bool drawArraysMode; // use glDrawArrays() if true. use glDrawElements() if false. (default: false)
	public:
		MeshGeometry();
		virtual ~MeshGeometry();

		void setDrawArraysMode(bool value) { drawArraysMode = value; }
		void draw();

		unsigned int getIndexCount();

		void updateVertexData(GLfloat* data, unsigned int length);
		void updateIndexData(GLuint* data, unsigned int length);
		void updateUVData(GLfloat* data, unsigned int length);
		void updateNormalData(GLfloat* data, unsigned int length);
		void updateTangentData(GLfloat* data, unsigned int length);
		void updateBitangentData(GLfloat* data, unsigned int length);

		// use same buffer with another MeshGeometry object.
		void burrowVertexBuffer(const MeshGeometry* other);
		void burrowNormalBuffer(const MeshGeometry* other);
		void burrowUVBuffer(const MeshGeometry* other);

	private:
		void calculateNormals_array();
		void calculateNormals_indexed();
	public:
		void calculateNormals();
		void calculateTangentBasis();

		void activatePositionBuffer(GLuint index);
		void activateUVBuffer(GLuint index);
		void activateNormalBuffer(GLuint index);
		void activateTangentBuffer(GLuint index);
		void activateBitangentBuffer(GLuint index);
		void activateIndexBuffer();

		void deactivatePositionBuffer(GLuint index);
		void deactivateUVBuffer(GLuint index);
		void deactivateNormalBuffer(GLuint index);
		void deactivateTangentBuffer(GLuint index);
		void deactivateBitangentBuffer(GLuint index);
		void deactivateIndexBuffer();

		GLuint getVertexLocation();
		GLuint getUVLocation();
		GLuint getNormalLocation();
		bool isVertexActivated();
		bool isUVActivated();
		bool isNormalActivated();

		//void applyTransform(glm::mat4 &transform);
		//void scale(float value);
		//void scaleUV(float scaleU, float scaleV);
		void dispose();
		//void convertToSeparateBuffers();
	};

}