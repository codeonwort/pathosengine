#include "deferredpass_pack_solidcolor.h"
#include "pathos/shader/shader_program.h"
#include "pathos/shader/shader.h"

#include "badger/types/vector_types.h"

namespace pathos {

	class SolidColorVS : public ShaderStage {
	public:
		SolidColorVS() : ShaderStage(GL_VERTEX_SHADER, "Material_SolidColorVS")
		{
			setFilepath("deferred_pack_solidcolor_vs.glsl");
		}
	};

	class SolidColorFS : public ShaderStage {
	public:
		SolidColorFS() : ShaderStage(GL_FRAGMENT_SHADER, "Material_SolidColorFS")
		{
			setFilepath("deferred_pack_solidcolor_fs.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_SolidColor, SolidColorVS, SolidColorFS);

}

namespace pathos {

	struct UBO_Deferred_Pack_SolidColor {
		matrix4 mvMatrix;
		matrix4 mvpMatrix;
		glm::mat3x4 mvMatrix3x3;
		vector4 albedo_metallic;
		vector4 emissive_roughness;
		vector4 billboardParam;
	};

	MeshDeferredRenderPass_Pack_SolidColor::MeshDeferredRenderPass_Pack_SolidColor() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_SolidColor::createProgram() {
		ShaderProgram& program1 = FIND_SHADER_PROGRAM(Program_SolidColor);
		program = program1.getGLName();
		
		ubo.init<UBO_Deferred_Pack_SolidColor>();
	}

	void MeshDeferredRenderPass_Pack_SolidColor::render(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* geometry,
		Material* inMaterial)
	{
		static_cast<void>(scene);
		ColorMaterial* material = static_cast<ColorMaterial*>(inMaterial);

		if (material->billboard) {
			geometry->activate_position_uv_normal(cmdList);
		} else {
			geometry->activate_position_normal(cmdList);
		}
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
