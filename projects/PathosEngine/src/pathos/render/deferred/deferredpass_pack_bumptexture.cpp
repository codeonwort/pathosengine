#include "deferredpass_pack_bumptexture.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	struct UBO_Deferred_Pack_BumpTexture {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::mat3 mvMatrix3x3;
	};

	static constexpr uint32_t DIFFUSE_TEXTURE_UNIT   = 0;
	static constexpr uint32_t NORMALMAP_TEXTURE_UNIT = 1;

	//static constexpr unsigned int SHADOW_MAPPING_TEXTURE_UNIT_START = 4;
	//static constexpr unsigned int OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START = 12;

	MeshDeferredRenderPass_Pack_BumpTexture::MeshDeferredRenderPass_Pack_BumpTexture() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_BumpTexture::createProgram() {
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Pack_BumpTexture");
		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Pack_BumpTexture");
		vs.loadSource("deferred_pack_bumptexture_vs.glsl");
		fs.loadSource("deferred_pack_bumptexture_fs.glsl");

		program = pathos::createProgram(vs, fs, "Deferred_Pack_BumpTexture");
		ubo.init<UBO_Deferred_Pack_BumpTexture>();
	}

	void MeshDeferredRenderPass_Pack_BumpTexture::render(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		BumpTextureMaterial* material = static_cast<BumpTextureMaterial*>(material_);

		geometry->activate_position_uv_normal_tangent_bitangent(cmdList);
		geometry->activateIndexBuffer(cmdList);

		UBO_Deferred_Pack_BumpTexture uboData;
		uboData.mvMatrix    = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix   = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.mvMatrix3x3 = glm::mat3(uboData.mvMatrix);
		ubo.update(cmdList, 1, &uboData);
		
		cmdList.bindTextureUnit(DIFFUSE_TEXTURE_UNIT, material->getDiffuseTexture());
		cmdList.bindTextureUnit(NORMALMAP_TEXTURE_UNIT, material->getNormalMapTexture());

		geometry->drawPrimitive(cmdList);
	}

}
