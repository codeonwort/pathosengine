#pragma once

#include "pathos/mesh/mesh.h"
#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/light/shadow.h"

namespace pathos {

	class MeshRenderPass {

	public:
		MeshRenderPass();
		virtual ~MeshRenderPass();

		inline void setModelMatrix(const glm::mat4& newModelMatrix) { modelMatrix = newModelMatrix; }
		inline void setShadowMapping(ShadowMap* shadow) { shadowMapping = shadow; }
		inline void setOmnidirectionalShadow(OmnidirectionalShadow* shadow) { omniShadow = shadow; }

		virtual void render(Scene*, Camera*, MeshGeometry*, Material*) = 0;

	protected:
		virtual void createProgram() = 0;

		void uploadDirectionalLightUniform(Scene* scene, uint32_t maxCount);
		void uploadPointLightUniform(Scene* scene, uint32_t maxCount);

		GLuint program; // name of shader program

		// @TODO: eliminate these?
		glm::mat4 modelMatrix;
		ShadowMap* shadowMapping = nullptr;
		OmnidirectionalShadow* omniShadow = nullptr;

		VertexShaderSource vsSource;
		FragmentShaderSource fsSource;

	};

}
