#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/matrix.hpp>
#include <pathos/core.h>

namespace pathos {

	class MeshGeometry : public NamedObject {
	private:
		GLfloat *positionData = nullptr, *uvData = nullptr, *normalData = nullptr, *tangentData = nullptr, *bitangentData = nullptr;
		GLuint* indexData = nullptr;
		GLuint positionBuffer = 0, uvBuffer = 0, normalBuffer = 0, indexBuffer = 0, tangentBuffer = 0, bitangentBuffer = 0;
		GLuint vertexLocation, uvLocation, normalLocation, tangentLocation, bitangentLocation; // xxLocation is valid only if xxActivated == true
		bool vertexActivated = false, uvActivated = false, normalActivated = false;
		unsigned int positionCount = 0, uvCount = 0, normalCount = 0, indexCount = 0; // it's array length, not actual count!
	public:
		MeshGeometry();
		virtual ~MeshGeometry();

		unsigned int getIndexCount();

		void updateVertexData(GLfloat* data, unsigned int length);
		void updateIndexData(GLuint* data, unsigned int length);
		void updateUVData(GLfloat* data, unsigned int length);
		void updateNormalData(GLfloat* data, unsigned int length);
		void updateTangentData(GLfloat* data, unsigned int length);
		void updateBitangentData(GLfloat* data, unsigned int length);

		inline GLfloat* getPos() { return positionData; }
		void uploadPosition();
		void calculateNormals();
		void calculateTangentBasis();

		void activateVertexBuffer(GLuint index);
		void activateUVBuffer(GLuint index);
		void activateNormalBuffer(GLuint index);
		void activateTangentBuffer(GLuint index);
		void activateBitangentBuffer(GLuint index);
		void activateIndexBuffer();

		void deactivateVertexBuffer(GLuint index);
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

		void applyTransform(glm::mat4 &transform);
		void scale(float value);
		void scaleUV(float scaleU, float scaleV);
		void dispose();
		//void convertToSeparateBuffers();
	};

	class PlaneGeometry : public MeshGeometry {
	private:
		float width, height;
		unsigned int gridX, gridY;
	public:
		PlaneGeometry(float width, float height, unsigned int segW = 1, unsigned int segH = 1);
		virtual void buildGeometry();
	};

	class CubeGeometry : public MeshGeometry {
	private:
		glm::vec3 halfSize;
	public:
		CubeGeometry(const glm::vec3& halfSize);
		virtual void buildGeometry();
	};

	class SphereGeometry : public MeshGeometry {
	private:
		float radius;
		unsigned int division;
	public:
		SphereGeometry(float radius, unsigned int division = 20);
		virtual void buildGeometry();
	};

}