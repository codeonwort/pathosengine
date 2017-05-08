#pragma once

#include "pathos/mesh/mesh.h"
#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/light/shadow.h"

namespace pathos {

	class MeshRenderPass {

	protected:
		GLuint program = 0; // shader program name

		// @TODO: eliminate these?
		glm::mat4 modelMatrix;
		ShadowMap* shadowMapping = nullptr;
		OmnidirectionalShadow* omniShadow = nullptr;

		VertexShaderSource vsSource;
		FragmentShaderSource fsSource;
		
		virtual void createProgram() = 0;
		void uploadDirectionalLightUniform(Scene*, unsigned int);
		void uploadPointLightUniform(Scene*, unsigned int);

	public:
		virtual ~MeshRenderPass();

		inline void setModelMatrix(const glm::mat4& modelMatrix) { this->modelMatrix = modelMatrix; }
		inline void setShadowMapping(ShadowMap* shadow) { shadowMapping = shadow; }
		inline void setOmnidirectionalShadow(OmnidirectionalShadow* shadow) { omniShadow = shadow; }

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) = 0;

	};

}