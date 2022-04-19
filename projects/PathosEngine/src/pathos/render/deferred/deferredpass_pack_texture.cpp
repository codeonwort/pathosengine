#include "deferredpass_pack_texture.h"
#include "glm/gtc/type_ptr.hpp"

static constexpr unsigned int TEXTURE_UNIT = 0;

namespace pathos {

	struct UBO_Deferred_Pack_Texture {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
	};

	MeshDeferredRenderPass_Pack_FlatTexture::MeshDeferredRenderPass_Pack_FlatTexture() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::createProgram() {
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Pack_FlatTexture");
		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Pack_FlatTexture");
		vs.loadSource("deferred_pack_texture_vs.glsl");
		fs.loadSource("deferred_pack_texture_fs.glsl");

		program = pathos::createProgram(vs, fs, "Deferred_Pack_FlatTexture");
		ubo.init<UBO_Deferred_Pack_Texture>();
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::render(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* geometry,
		Material* inMaterial)
	{
		static_cast<void>(scene);
		TextureMaterial* material = static_cast<TextureMaterial*>(inMaterial);

		geometry->activate_position_uv_normal(cmdList);
		geometry->activateIndexBuffer(cmdList);

		// uniform: transform
		UBO_Deferred_Pack_Texture uboData;
		uboData.mvMatrix = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		ubo.update(cmdList, 1, &uboData);

		// texture binding
		cmdList.bindTextureUnit(TEXTURE_UNIT, material->getTexture());

		geometry->drawPrimitive(cmdList);
	}

}
