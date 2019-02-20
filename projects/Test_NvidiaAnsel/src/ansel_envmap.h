#pragma once

#include "gl_core.h"
#include "pathos/mesh/geometry.h"

namespace pathos {

	class IcosahedronGeometry : public MeshGeometry {

		static constexpr uint32_t INITIAL_NUM_VERTICES = 12;
		static constexpr uint32_t INITIAL_NUM_TRIANLGES = 20;
		
	public:
		IcosahedronGeometry(uint32_t subdivisionStep = 0);

	private:
		// subroutines
		void buildGeometry();		// initial positions and indices of icosahedron
		void subdivide();			// subdivision step
		void uploadToGPU();			// GL calls

		GLfloat* positionData;
		GLfloat* uvData;
		GLfloat* normalData;
		GLuint* indexData;

		uint32_t numVertices;
		uint32_t numTriangles;
		
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