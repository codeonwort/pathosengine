#include "deferredpass_pack_solidcolor.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	struct UBO_Deferred_Pack_SolidColor {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::vec4 albedo;
		glm::vec4 metallic_roughness;
	};

	MeshDeferredRenderPass_Pack_SolidColor::MeshDeferredRenderPass_Pack_SolidColor() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_SolidColor::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_solidcolor_vs.glsl");
		fs.loadSource("deferred_pack_solidcolor_fs.glsl");

		program = pathos::createProgram(vs, fs);
		ubo.init<UBO_Deferred_Pack_SolidColor>();
	}

	void MeshDeferredRenderPass_Pack_SolidColor::render(Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		static_cast<void>(scene);
		ColorMaterial* material = static_cast<ColorMaterial*>(material_);

		geometry->activate_position_normal();
		geometry->activateIndexBuffer();

		UBO_Deferred_Pack_SolidColor uboData;
		uboData.mvMatrix             = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix            = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.albedo.x             = material->getAlbedo()[0];
		uboData.albedo.y             = material->getAlbedo()[1];
		uboData.albedo.z             = material->getAlbedo()[2];
		uboData.albedo.w             = 0.0f;
		uboData.metallic_roughness.x = material->getMetallic();
		uboData.metallic_roughness.y = material->getRoughness();
		ubo.update(1, &uboData);

		geometry->draw();
	}

}
