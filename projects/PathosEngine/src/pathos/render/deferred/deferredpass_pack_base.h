#pragma once

//#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"
#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class MeshDeferredRenderPass_Pack {

	public:
		virtual ~MeshDeferredRenderPass_Pack();

		// #todo-refactoring: Remove this stupid function
		inline void setModelMatrix(const glm::mat4& newModelMatrix) { modelMatrix = newModelMatrix; }

		void bindProgram(RenderCommandList& cmdList);

		virtual void render(RenderCommandList& cmdList, Scene* inScene, Camera* inCamera, MeshGeometry* inMesh, Material* inMaterial) = 0;

	protected:
		virtual void createProgram() = 0;

		GLuint program = 0; // shader program name

		// #todo-refactoring: eliminate this
		glm::mat4 modelMatrix;

	};

}
