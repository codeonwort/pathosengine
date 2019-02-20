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

#define GET_UNIFORM(z) { uniform_##z = glGetUniformLocation(program, #z); assert(uniform_##z != -1); }
		GET_UNIFORM(mvTransform);
		GET_UNIFORM(mvpTransform);
		GET_UNIFORM(tex_diffuse);
#undef GET_UNIFORM
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		TextureMaterial* material = static_cast<TextureMaterial*>(material_);

		geometry->activate_position_uv_normal();
		geometry->activateIndexBuffer();

		// uniform: transform
		const glm::mat4& mvMatrix = camera->getViewMatrix() * modelMatrix;
		const glm::mat4& mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		glUniformMatrix4fv(uniform_mvTransform, 1, false, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(mvpMatrix));

		// texture binding
		glBindTextureUnit(TEXTURE_UNIT, material->getTexture());

		geometry->draw();

		geometry->deactivate();
		geometry->deactivateIndexBuffer();
	}

}