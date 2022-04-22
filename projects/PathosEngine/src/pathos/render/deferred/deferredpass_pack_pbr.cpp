#include "deferredpass_pack_pbr.h"
#include "pathos/shader/shader_program.h"
#include "pathos/shader/shader_program.h"

#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	class DefaultLitVS : public ShaderStage {
	public:
		DefaultLitVS() : ShaderStage(GL_VERTEX_SHADER, "Material_DefaultLitVS")
		{
			setFilepath("deferred_pack_pbr_vs.glsl");
		}
	};

	template<bool TriplanarMapping>
	class DefaultLitFS : public ShaderStage {
	public:
		DefaultLitFS() : ShaderStage(GL_FRAGMENT_SHADER, "Material_DefaultLitFS")
		{
			if (TriplanarMapping) {
				addDefine("TRIPLANAR_MAPPING", 1);
			}
			setFilepath("deferred_pack_pbr_fs.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_DefaultLit, DefaultLitVS, DefaultLitFS<false>);
	DEFINE_SHADER_PROGRAM2(Program_DefaultLitTriplanarMapping, DefaultLitVS, DefaultLitFS<true>);
}

namespace pathos {

	struct UBO_Deferred_Pack_PBR {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::mat3x4 mvMatrix3x3;
	};

	static constexpr uint32 ALBEDO_TEXTURE_UNIT    = 0;
	static constexpr uint32 NORMAL_TEXTURE_UNIT    = 1;
	static constexpr uint32 METALLIC_TEXTURE_UNIT  = 2;
	static constexpr uint32 ROUGHNESS_TEXTURE_UNIT = 3;
	static constexpr uint32 AO_TEXTURE_UNIT        = 4;

	MeshDeferredRenderPass_Pack_PBR::MeshDeferredRenderPass_Pack_PBR() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_PBR::createProgram() {
		ShaderProgram& program1 = FIND_SHADER_PROGRAM(Program_DefaultLit);
		program = program1.getGLName();

		ubo.init<UBO_Deferred_Pack_PBR>();
	}

	void MeshDeferredRenderPass_Pack_PBR::render(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* geometry,
		Material* inMaterial)
	{
		PBRTextureMaterial* material = static_cast<PBRTextureMaterial*>(inMaterial);

		// #todo-material: hack and it increases overhead of shader changes.
		if (material->useTriplanarMapping) {
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DefaultLitTriplanarMapping);
			cmdList.useProgram(program.getGLName());
		} else {
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DefaultLit);
			cmdList.useProgram(program.getGLName());
		}

		geometry->activate_position_uv_normal_tangent_bitangent(cmdList);
		geometry->activateIndexBuffer(cmdList);

		// uniform: transform
		UBO_Deferred_Pack_PBR uboData;
		uboData.mvMatrix    = camera->getViewMatrix() * modelMatrix;
		uboData.mvpMatrix   = camera->getViewProjectionMatrix() * modelMatrix;
		uboData.mvMatrix3x3 = glm::mat3x4(uboData.mvMatrix);
		ubo.update(cmdList, 1, &uboData);

		// uniform: texture
		cmdList.bindTextureUnit(ALBEDO_TEXTURE_UNIT, material->getAlbedo());
		cmdList.bindTextureUnit(NORMAL_TEXTURE_UNIT, material->getNormal());
		cmdList.bindTextureUnit(METALLIC_TEXTURE_UNIT, material->getMetallic());
		cmdList.bindTextureUnit(ROUGHNESS_TEXTURE_UNIT, material->getRoughness());
		cmdList.bindTextureUnit(AO_TEXTURE_UNIT, material->getAO());

		geometry->drawPrimitive(cmdList);
	}

}
