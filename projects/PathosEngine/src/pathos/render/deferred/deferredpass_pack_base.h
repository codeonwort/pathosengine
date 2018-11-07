#pragma once

//#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"
#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
//#include "pathos/light/shadow.h"

namespace pathos {

	class MeshDeferredRenderPass_Pack {

	protected:
		GLuint program = 0; // shader program name

		// @TODO: eliminate this?
		glm::mat4 modelMatrix;

		virtual void createProgram() = 0;

	public:
		virtual ~MeshDeferredRenderPass_Pack();

		inline void setModelMatrix(const glm::mat4& newModelMatrix) { modelMatrix = newModelMatrix; }

		void bindProgram();
		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) = 0;

	};

}