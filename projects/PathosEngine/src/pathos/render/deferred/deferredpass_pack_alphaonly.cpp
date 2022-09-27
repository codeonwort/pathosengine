#include "deferredpass_pack_alphaonly.h"
#include "pathos/shader/shader_program.h"

#include "badger/types/matrix_types.h"
#include "badger/types/vector_types.h"

namespace pathos {

	static constexpr uint32 UBO_BINDING_INDEX = 1;
	static constexpr uint32 ALBEDO_TEXTURE_UNIT = 0;

	struct UBO_Deferred_Pack_AlphaOnly {
		matrix4 modelView;
		matrix3x4 modelView3x3;
		vector4 color;
	};

	class DeferredPackAlphaOnlyVS : public ShaderStage {
	public:
		DeferredPackAlphaOnlyVS() : ShaderStage(GL_VERTEX_SHADER, "DeferredPackAlphaOnlyVS") {
			addDefine("VERTEX_SHADER", 1);
			setFilepath("deferred_pack_alphaonly.glsl");
		}
	};

	class DeferredPackAlphaOnlyFS : public ShaderStage {
	public:
		DeferredPackAlphaOnlyFS() : ShaderStage(GL_FRAGMENT_SHADER, "DeferredPackAlphaOnlyFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("deferred_pack_alphaonly.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_Deferred_Pack_AlphaOnly, DeferredPackAlphaOnlyVS, DeferredPackAlphaOnlyFS);
}

namespace pathos {

	MeshDeferredRenderPass_Pack_AlphaOnly::MeshDeferredRenderPass_Pack_AlphaOnly() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_AlphaOnly::render(
		RenderCommandList& cmdList,
		SceneProxy* inScene,
		Camera* inCamera,
		MeshGeometry* inMesh,
		Material* inMaterial)
	{
		AlphaOnlyTextureMaterial* material = static_cast<AlphaOnlyTextureMaterial*>(inMaterial);
		const float* colorPtr = material->getColor();

		// Bind uniform
		UBO_Deferred_Pack_AlphaOnly uboData;
		uboData.modelView     = inCamera->getViewMatrix() * modelMatrix;
		uboData.modelView3x3  = matrix3x4(uboData.modelView);
		uboData.color         = vector4(colorPtr[0], colorPtr[1], colorPtr[2], 1.0f);
		ubo.update(cmdList, UBO_BINDING_INDEX, &uboData);

		// Bind texture
		cmdList.bindTextureUnit(ALBEDO_TEXTURE_UNIT, material->getTexture());

		// Drawcall
		inMesh->activate_position_uv_normal_tangent_bitangent(cmdList);
		inMesh->activateIndexBuffer(cmdList);
		inMesh->drawPrimitive(cmdList);
	}

	void MeshDeferredRenderPass_Pack_AlphaOnly::createProgram() {
		ShaderProgram& program1 = FIND_SHADER_PROGRAM(Program_Deferred_Pack_AlphaOnly);
		program = program1.getGLName();

		ubo.init<UBO_Deferred_Pack_AlphaOnly>();
	}

}
