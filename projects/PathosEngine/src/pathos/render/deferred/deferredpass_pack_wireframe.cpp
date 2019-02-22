#include "deferredpass_pack_wireframe.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	struct UBO_Deferred_Pack_Wireframe {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::vec3 diffuseColor;
	};

	MeshDeferredRenderPass_Pack_Wireframe::MeshDeferredRenderPass_Pack_Wireframe() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_Wireframe::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_wireframe_vs.glsl");
		fs.loadSource("deferred_pack_wireframe_fs.glsl");

		program = pathos::createProgram(vs, fs);
		ubo.init<UBO_Deferred_Pack_Wireframe>();
	}

	void MeshDeferredRenderPass_Pack_Wireframe::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		WireframeMaterial* material = static_cast<WireframeMaterial*>(material_);

		geometry->activate_position_normal();
		geometry->activateIndexBuffer();

		UBO_Deferred_Pack_Wireframe uboData;
		uboData.mvMatrix       = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix      = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.diffuseColor.x = material->getColor()[0];
		uboData.diffuseColor.y = material->getColor()[1];
		uboData.diffuseColor.z = material->getColor()[2];
		ubo.update(1, &uboData);

		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		geometry->draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);

		geometry->deactivate();
		geometry->deactivateIndexBuffer();
	}

}
