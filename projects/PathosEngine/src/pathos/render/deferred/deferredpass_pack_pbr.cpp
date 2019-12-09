#include "deferredpass_pack_pbr.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	struct UBO_Deferred_Pack_PBR {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::mat3 mvMatrix3x3;
	};

	static constexpr uint32 ALBEDO_TEXTURE_UNIT    = 0;
	static constexpr uint32 NORMAL_TEXTURE_UNIT    = 1;
	static constexpr uint32 METALLIC_TEXTURE_UNIT  = 2;
	static constexpr uint32 ROUGHNESS_TEXTURE_UNIT = 3;
	static constexpr uint32 AO_TEXTURE_UNIT        = 4;

	MeshDeferredRenderPass_Pack_PBR::MeshDeferredRenderPass_Pack_PBR() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_PBR::createProgram() {
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Pack_PBR");
		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Pack_PBR");
		vs.loadSource("deferred_pack_pbr_vs.glsl");
		fs.loadSource("deferred_pack_pbr_fs.glsl");

		program = pathos::createProgram(vs, fs, "Deferred_Pack_PBR");
		ubo.init<UBO_Deferred_Pack_PBR>();
	}

	void MeshDeferredRenderPass_Pack_PBR::render(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		PBRTextureMaterial* material = static_cast<PBRTextureMaterial*>(material_);

		geometry->activate_position_uv_normal_tangent_bitangent(cmdList);
		geometry->activateIndexBuffer(cmdList);

		// uniform: transform
		UBO_Deferred_Pack_PBR uboData;
		uboData.mvMatrix    = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix   = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.mvMatrix3x3 = glm::mat3(uboData.mvMatrix);
		ubo.update(cmdList, 1, &uboData);

		// uniform: texture
		cmdList.bindTextureUnit(ALBEDO_TEXTURE_UNIT, material->getAlbedo());
		cmdList.bindTextureUnit(NORMAL_TEXTURE_UNIT, material->getNormal());
		cmdList.bindTextureUnit(METALLIC_TEXTURE_UNIT, material->getMetallic());
		cmdList.bindTextureUnit(ROUGHNESS_TEXTURE_UNIT, material->getRoughness());
		cmdList.bindTextureUnit(AO_TEXTURE_UNIT, material->getAO());

		geometry->drawPrimitive(cmdList);
	}

}
