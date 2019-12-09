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
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Pack_Wireframe");
		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Pack_Wireframe");
		vs.loadSource("deferred_pack_wireframe_vs.glsl");
		fs.loadSource("deferred_pack_wireframe_fs.glsl");

		program = pathos::createProgram(vs, fs, "Deferred_Pack_Wireframe");
		ubo.init<UBO_Deferred_Pack_Wireframe>();
	}

	void MeshDeferredRenderPass_Pack_Wireframe::render(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		static_cast<void>(scene);
		WireframeMaterial* material = static_cast<WireframeMaterial*>(material_);

		geometry->activate_position_normal(cmdList);
		geometry->activateIndexBuffer(cmdList);

		UBO_Deferred_Pack_Wireframe uboData;
		uboData.mvMatrix       = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix      = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.diffuseColor.x = material->getColor()[0];
		uboData.diffuseColor.y = material->getColor()[1];
		uboData.diffuseColor.z = material->getColor()[2];
		ubo.update(cmdList, 1, &uboData);

		cmdList.disable(GL_CULL_FACE);
		cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
		geometry->drawPrimitive(cmdList);
		cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
		cmdList.enable(GL_CULL_FACE);
	}

}
