#pragma once

#include "sky.h"
#include "pathos/mesh/geometry.h"
#include "gl_core.h"

namespace pathos {

	class IcosahedronGeometry : public MeshGeometry {

		static constexpr uint32_t INITIAL_NUM_VERTICES = 12;
		static constexpr uint32_t INITIAL_NUM_TRIANLGES = 20;
		
	public:
		IcosahedronGeometry(uint32_t subdivisionStep = 0);

	private:
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

	class AnselSkyRendering : public SkyRendering {
		
	public:
		AnselSkyRendering(GLuint textureID);
		~AnselSkyRendering();

		void render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) override;

	private:
		GLint uniform_transform = 0;
		GLuint texture = 0;

		MeshGeometry* sphere = nullptr;

	};

}
