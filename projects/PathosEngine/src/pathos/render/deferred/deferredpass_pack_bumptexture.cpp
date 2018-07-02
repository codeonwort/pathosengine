#include "deferredpass_pack_bumptexture.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	static constexpr unsigned int DIFFUSE_TEXTURE_UNIT = 0;
	static constexpr unsigned int NORMALMAP_TEXTURE_UNIT = 1;

	//static constexpr unsigned int SHADOW_MAPPING_TEXTURE_UNIT_START = 4;
	//static constexpr unsigned int OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START = 12;

	MeshDeferredRenderPass_Pack_BumpTexture::MeshDeferredRenderPass_Pack_BumpTexture() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_BumpTexture::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_bumptexture_vs.glsl");
		fs.loadSource("deferred_pack_bumptexture_fs.glsl");

		program = pathos::createProgram(vs, fs);

#define GET_UNIFORM(z) { uniform_##z = glGetUniformLocation(program, #z); assert(uniform_##z != -1); }
		GET_UNIFORM(mvTransform3x3);
		GET_UNIFORM(mvTransform);
		GET_UNIFORM(mvpTransform);
#undef GET_UNIFORM
	}

	void MeshDeferredRenderPass_Pack_BumpTexture::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		BumpTextureMaterial* material = static_cast<BumpTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_uv_normal_tangent_bitangent();
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// uniform: transform
		glm::mat4 mvMatrix = camera->getViewMatrix() * modelMatrix;
		glm::mat3 mvMatrix3x3 = glm::mat3(mvMatrix);
		glm::mat4 mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		glUniformMatrix3fv(uniform_mvTransform3x3, 1, false, glm::value_ptr(mvMatrix3x3));
		glUniformMatrix4fv(uniform_mvTransform, 1, false, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(mvpMatrix));

		// uniform: texture
		glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getDiffuseTexture());
		glActiveTexture(GL_TEXTURE0 + NORMALMAP_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getNormalMapTexture());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivate();
		geometry->deactivateIndexBuffer();

		glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + NORMALMAP_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

}