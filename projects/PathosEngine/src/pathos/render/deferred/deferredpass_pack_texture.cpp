#include "deferredpass_pack_texture.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	MeshDeferredRenderPass_Pack_FlatTexture::MeshDeferredRenderPass_Pack_FlatTexture() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_texture_vs.glsl");
		fs.loadSource("deferred_pack_texture_fs.glsl");
		program = pathos::createProgram(vs, fs);

		positionLocation = 0;
		uvLocation = 1;
		normalLocation = 2;
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		TextureMaterial* material = static_cast<TextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activatePositionBuffer(positionLocation);
		geometry->activateUVBuffer(uvLocation);
		geometry->activateNormalBuffer(normalLocation);
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// uniform: transform
		const glm::mat4& mvMatrix = camera->getViewMatrix() * modelMatrix;
		const glm::mat4& mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(program, "mvTransform"), 1, false, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(mvpMatrix));

		// texture binding
		glActiveTexture(GL_TEXTURE0 + MeshDeferredRenderPass_Pack_FlatTexture::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getTexture());
		glUniform1i(glGetUniformLocation(program, "tex_diffuse"), TEXTURE_UNIT);

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivatePositionBuffer(positionLocation);
		geometry->deactivateUVBuffer(uvLocation);
		geometry->deactivateNormalBuffer(normalLocation);
		geometry->deactivateIndexBuffer();

		glActiveTexture(GL_TEXTURE0 + MeshDeferredRenderPass_Pack_FlatTexture::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

}