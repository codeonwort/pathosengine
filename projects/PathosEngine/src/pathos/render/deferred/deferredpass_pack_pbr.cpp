#include "deferredpass_pack_pbr.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	namespace {
		static constexpr unsigned int ALBEDO_TEXTURE_UNIT = 0;
		static constexpr unsigned int NORMAL_TEXTURE_UNIT = 1;
		static constexpr unsigned int METALLIC_TEXTURE_UNIT = 2;
		static constexpr unsigned int ROUGHNESS_TEXTURE_UNIT = 3;
		static constexpr unsigned int AO_TEXTURE_UNIT = 4;
	}

	MeshDeferredRenderPass_Pack_PBR::MeshDeferredRenderPass_Pack_PBR() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_PBR::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_pbr_vs.glsl");
		fs.loadSource("deferred_pack_pbr_fs.glsl");

		program = pathos::createProgram(vs, fs);

#define GET_UNIFORM(z) { uniform_##z = glGetUniformLocation(program, #z); assert(uniform_##z != -1); }
		GET_UNIFORM(mvTransform3x3);
		GET_UNIFORM(mvTransform);
		GET_UNIFORM(mvpTransform);
#undef GET_UNIFORM
	}

	void MeshDeferredRenderPass_Pack_PBR::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		PBRTextureMaterial* material = static_cast<PBRTextureMaterial*>(material_);
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
		glActiveTexture(GL_TEXTURE0 + ALBEDO_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getAlbedo());
		glActiveTexture(GL_TEXTURE0 + NORMAL_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getNormal());
		glActiveTexture(GL_TEXTURE0 + METALLIC_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getMetallic());
		glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getRoughness());
		glActiveTexture(GL_TEXTURE0 + AO_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getAO());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivate();
		geometry->deactivateIndexBuffer();

		glActiveTexture(GL_TEXTURE0 + ALBEDO_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + NORMAL_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + METALLIC_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + AO_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

}