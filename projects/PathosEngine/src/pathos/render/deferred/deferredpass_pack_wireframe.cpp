#include "deferredpass_pack_wireframe.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	MeshDeferredRenderPass_Pack_Wireframe::MeshDeferredRenderPass_Pack_Wireframe() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_Wireframe::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_wireframe_vs.glsl");
		fs.loadSource("deferred_pack_wireframe_fs.glsl");

		program = pathos::createProgram(vs, fs);

#define GET_UNIFORM(z) { uniform_##z = glGetUniformLocation(program, #z); assert(uniform_##z != -1); }
		GET_UNIFORM(mvTransform);
		GET_UNIFORM(mvpTransform);
		GET_UNIFORM(diffuseColor);
#undef GET_UNIFORM
	}

	void MeshDeferredRenderPass_Pack_Wireframe::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		WireframeMaterial* material = static_cast<WireframeMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_normal();
		geometry->activateIndexBuffer();

		const glm::mat4& mvMatrix = camera->getViewMatrix() * modelMatrix;
		const glm::mat4& mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		glUniformMatrix4fv(uniform_mvTransform, 1, false, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(mvpMatrix));
		glUniform3fv(uniform_diffuseColor, 1, material->getColor());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		geometry->draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivate();
		geometry->deactivateIndexBuffer();
	}

}