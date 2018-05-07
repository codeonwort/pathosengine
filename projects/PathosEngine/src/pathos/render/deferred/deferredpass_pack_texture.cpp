#include "deferredpass_pack_texture.h"
#include "glm/gtc/type_ptr.hpp"

static constexpr unsigned int TEXTURE_UNIT = 0;

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

#define GET_UNIFORM(z) { assert((uniform_##z = glGetUniformLocation(program, #z)) != -1); }
		GET_UNIFORM(mvTransform);
		GET_UNIFORM(mvpTransform);
		GET_UNIFORM(tex_diffuse);
#undef GET_UNIFORM

		positionLocation = 0;
		uvLocation = 1;
		normalLocation = 2;
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
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
		glUniformMatrix4fv(uniform_mvTransform, 1, false, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(mvpMatrix));

		// texture binding
		glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getTexture());
		glUniform1i(uniform_tex_diffuse, TEXTURE_UNIT);

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

		glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

}