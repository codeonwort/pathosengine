#include "deferredpass_pack_solidcolor.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	struct UBO_Deferred_Pack_SolidColor {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::vec3 diffuseColor;
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
		uboData.mvMatrix       = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix      = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.diffuseColor.x = material->getDiffuse()[0];
		uboData.diffuseColor.y = material->getDiffuse()[1];
		uboData.diffuseColor.z = material->getDiffuse()[2];
		ubo.update(1, &uboData);

		geometry->draw();

		geometry->deactivate();
		geometry->deactivateIndexBuffer();
	}

}
