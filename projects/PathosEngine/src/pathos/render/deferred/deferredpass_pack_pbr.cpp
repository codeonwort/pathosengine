#include "deferredpass_pack_pbr.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	static constexpr unsigned int ALBEDO_TEXTURE_UNIT    = 0;
	static constexpr unsigned int NORMAL_TEXTURE_UNIT    = 1;
	static constexpr unsigned int METALLIC_TEXTURE_UNIT  = 2;
	static constexpr unsigned int ROUGHNESS_TEXTURE_UNIT = 3;
	static constexpr unsigned int AO_TEXTURE_UNIT        = 4;

	MeshDeferredRenderPass_Pack_PBR::MeshDeferredRenderPass_Pack_PBR() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_PBR::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_pbr_vs.glsl");
		fs.loadSource("deferred_pack_pbr_fs.glsl");

		program = pathos::createProgram(vs, fs);
	}

	void MeshDeferredRenderPass_Pack_PBR::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		PBRTextureMaterial* material = static_cast<PBRTextureMaterial*>(material_);

		geometry->activate_position_uv_normal_tangent_bitangent();
		geometry->activateIndexBuffer();

		// uniform: transform
		UBO_Deferred_Pack_PBR uboData;
		uboData.mvMatrix    = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix   = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.mvMatrix3x3 = glm::mat3(uboData.mvMatrix);
		ubo.update(1, uboData);

		// uniform: texture
		glBindTextureUnit(ALBEDO_TEXTURE_UNIT, material->getAlbedo());
		glBindTextureUnit(NORMAL_TEXTURE_UNIT, material->getNormal());
		glBindTextureUnit(METALLIC_TEXTURE_UNIT, material->getMetallic());
		glBindTextureUnit(ROUGHNESS_TEXTURE_UNIT, material->getRoughness());
		glBindTextureUnit(AO_TEXTURE_UNIT, material->getAO());

		geometry->draw();

		geometry->deactivate();
		geometry->deactivateIndexBuffer();
	}

}
