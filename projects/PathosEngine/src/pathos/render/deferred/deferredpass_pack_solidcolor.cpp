#include "deferredpass_pack_solidcolor.h"
#include "pathos/shader/shader_program.h"

#include "badger/types/vector_types.h"

// #todo-shader: Use this instead
//namespace pathos {
//
//	class SolidColorVS : public ShaderStage {
//		SolidColorVS() : ShaderStage(GL_VERTEX_SHADER, "SolidColorVS")
//		{
//			setFilepath("deferred_pack_solidcolor_vs.glsl");
//		}
//	};
//
//	class SolidColorFS : public ShaderStage {
//		SolidColorFS() : ShaderStage(GL_FRAGMENT_SHADER, "SolidColorFS")
//		{
//			setFilepath("deferred_pack_solidcolor_fs.glsl");
//		}
//	};
//
//	DEFINE_SHADER_PROGRAM2(Program_SolidColor, SolidColorVS, SolidColorFS);
//	DEFINE_SHADER_PROGRAM2(Program_BillboardSolidColor, BillboardSolidColorVS, SolidColorFS);
//
//}

namespace pathos {

	struct UBO_Deferred_Pack_SolidColor {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::mat3x4 mvMatrix3x3;
		glm::vec4 albedo_metallic;
		glm::vec4 emissive_roughness;
		glm::vec4 billboardParam;
	};

	MeshDeferredRenderPass_Pack_SolidColor::MeshDeferredRenderPass_Pack_SolidColor() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_SolidColor::createProgram() {
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Pack_SolidColor");
		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Pack_SolidColor");
		vs.loadSource("deferred_pack_solidcolor_vs.glsl");
		fs.loadSource("deferred_pack_solidcolor_fs.glsl");

		program = pathos::createProgram(vs, fs, "Deferred_Pack_SolidColor");
		ubo.init<UBO_Deferred_Pack_SolidColor>();
	}

	void MeshDeferredRenderPass_Pack_SolidColor::render(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* geometry, Material* inMaterial) {
		static_cast<void>(scene);
		ColorMaterial* material = static_cast<ColorMaterial*>(inMaterial);

		geometry->activate_position_uv_normal(cmdList);
		geometry->activateIndexBuffer(cmdList);

		UBO_Deferred_Pack_SolidColor uboData;
		uboData.mvMatrix             = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix            = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.mvMatrix3x3          = glm::mat3x4(uboData.mvMatrix);
		uboData.albedo_metallic      = vector4(material->getAlbedo(), material->getMetallic());
		uboData.emissive_roughness   = vector4(material->getEmissive(), material->getRoughness());
		// #todo-material: temp billboard
		uboData.billboardParam.x     = material->billboard ? 1.0f : 0.0f;
		uboData.billboardParam.y     = material->billboardWidth;
		ubo.update(cmdList, 1, &uboData);

		geometry->drawPrimitive(cmdList);
	}

}
