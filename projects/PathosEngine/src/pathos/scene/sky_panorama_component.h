#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/mesh/geometry.h"
#include "pathos/scene/scene_component.h"

namespace pathos {

	// #todo-refactoring: Move to another folder
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

}

namespace pathos {

	struct PanoramaSkyProxy : SceneComponentProxy {
		MeshGeometry* sphere;
		GLuint textureID;
	};

	class PanoramaSkyComponent : public SceneComponent {

	public:
		~PanoramaSkyComponent();

		void initialize(GLuint textureID);

		inline bool hasValidResources() const {
			return textureID != 0 && sphere != nullptr;
		}

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		GLuint textureID = 0;
		MeshGeometry* sphere = nullptr;

	};

}
