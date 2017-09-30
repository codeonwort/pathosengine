#pragma once

#include <GL/glew.h>
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	class IcosahedronGeometry : public MeshGeometry {

		static constexpr unsigned int INITIAL_NUM_VERTICES = 12;
		static constexpr unsigned int INITIAL_NUM_TRIANLGES = 20;
		
	public:
		IcosahedronGeometry(unsigned int subdivisionStep = 0);

	private:
		// subroutines
		void buildGeometry();		// initial positions and indices of icosahedron
		void subdivide();			// subdivision step
		//void buildNormalAndUV();	// invoked after all subdivisions
		void uploadToGPU();			// GL calls

		GLfloat* positionData;
		GLfloat* uvData;
		GLfloat* normalData;
		GLuint* indexData;

		unsigned int numVertices;
		unsigned int numTriangles;
		
	};

	// alternative for Skybox
	class AnselEnvMapping {
		
	public:
		AnselEnvMapping(GLuint textureID);
		~AnselEnvMapping();

		// transform: view-projection transform without camera transition
		void render(const glm::mat4& transform);

	private:
		void createShaderProgram();

		GLuint texture = 0xFFFFFFFF;
		GLuint program = 0;
		GLint uniform_transform = -1;
		MeshGeometry* sphere = nullptr;

	};

}