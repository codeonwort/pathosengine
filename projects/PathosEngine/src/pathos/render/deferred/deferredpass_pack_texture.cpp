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
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_texture_vs.glsl");
		fs.loadSource("deferred_pack_texture_fs.glsl");

		program = pathos::createProgram(vs, fs);
		ubo.init<UBO_Deferred_Pack_Texture>();
	}

	void MeshDeferredRenderPass_Pack_FlatTexture::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		TextureMaterial* material = static_cast<TextureMaterial*>(material_);

		geometry->activate_position_uv_normal();
		geometry->activateIndexBuffer();

		// uniform: transform
		UBO_Deferred_Pack_Texture uboData;
		uboData.mvMatrix = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		ubo.update(1, &uboData);

		// texture binding
		glBindTextureUnit(TEXTURE_UNIT, material->getTexture());

		geometry->draw();
	}

}
