#include "deferredpass_pack_wireframe.h"
#include "pathos/shader/shader_program.h"

#include "badger/types/matrix_types.h"
#include "badger/types/vector_types.h"

namespace pathos {

	struct UBO_Deferred_Pack_Wireframe {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4 mvMatrix;
		vector3 diffuseColor;
	};

	class DeferredPackWireframeVS : public ShaderStage {
	public:
		DeferredPackWireframeVS() : ShaderStage(GL_VERTEX_SHADER, "DeferredPackWireframeVS") {
			addDefine("VERTEX_SHADER", 1);
			setFilepath("deferred_pack_wireframe_vs.glsl");
		}
	};

	class DeferredPackWireframeFS : public ShaderStage {
	public:
		DeferredPackWireframeFS() : ShaderStage(GL_FRAGMENT_SHADER, "DeferredPackWireframeFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("deferred_pack_wireframe_fs.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_Deferred_Pack_Wireframe, DeferredPackWireframeVS, DeferredPackWireframeFS);
}

namespace pathos {

	MeshDeferredRenderPass_Pack_Wireframe::MeshDeferredRenderPass_Pack_Wireframe() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_Wireframe::createProgram() {
		ShaderProgram& program1 = FIND_SHADER_PROGRAM(Program_Deferred_Pack_Wireframe);
		program = program1.getGLName();

		ubo.init<UBO_Deferred_Pack_Wireframe>();
	}

	void MeshDeferredRenderPass_Pack_Wireframe::render(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* geometry,
		Material* inMaterial)
	{
		static_cast<void>(scene);
		WireframeMaterial* material = static_cast<WireframeMaterial*>(inMaterial);

		geometry->activate_position_normal(cmdList);
		geometry->activateIndexBuffer(cmdList);

		UBO_Deferred_Pack_Wireframe uboData;
		uboData.mvMatrix       = camera->getViewMatrix() * modelMatrix;
		uboData.diffuseColor.x = material->getColor()[0];
		uboData.diffuseColor.y = material->getColor()[1];
		uboData.diffuseColor.z = material->getColor()[2];
		ubo.update(cmdList, UBO_Deferred_Pack_Wireframe::BINDING_POINT, &uboData);

		cmdList.disable(GL_CULL_FACE);
		cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

		geometry->drawPrimitive(cmdList);

		cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
		cmdList.enable(GL_CULL_FACE);
	}

}
