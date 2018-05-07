#include "deferredpass_pack_solidcolor.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	MeshDeferredRenderPass_Pack_SolidColor::MeshDeferredRenderPass_Pack_SolidColor() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_SolidColor::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_solidcolor_vs.glsl");
		fs.loadSource("deferred_pack_solidcolor_fs.glsl");

		program = pathos::createProgram(vs, fs);

#define GET_UNIFORM(z) { assert((uniform_##z = glGetUniformLocation(program, #z)) != -1); }
		GET_UNIFORM(mvTransform);
		GET_UNIFORM(mvpTransform);
		GET_UNIFORM(diffuseColor);
#undef GET_UNIFORM

		positionLocation = 0;
		normalLocation = 2;
	}

	void MeshDeferredRenderPass_Pack_SolidColor::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		ColorMaterial* material = static_cast<ColorMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activatePositionBuffer(positionLocation);
		geometry->activateNormalBuffer(normalLocation);
		geometry->activateIndexBuffer();

		glUseProgram(program);

		const glm::mat4& mvMatrix = camera->getViewMatrix() * modelMatrix;
		const glm::mat4& mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		glUniformMatrix4fv(uniform_mvTransform, 1, false, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(mvpMatrix));
		//glUniform3fv(uniform_ambientColor, 1, material->getAmbient());
		glUniform3fv(uniform_diffuseColor, 1, material->getDiffuse());
		//glUniform3fv(uniform_specularColor, 1, material->getSpecular());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivatePositionBuffer(positionLocation);
		geometry->deactivateNormalBuffer(normalLocation);
		geometry->deactivateIndexBuffer();

		glUseProgram(0);
	}

}