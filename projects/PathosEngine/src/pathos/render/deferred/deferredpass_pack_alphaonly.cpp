#include "deferredpass_pack_alphaonly.h"
#include "pathos/shader/shader_program.h"

#include "badger/types/matrix_types.h"
#include "badger/types/vector_types.h"

// #todo: Cannot enable this due to static initialization order.
#define ALPHAONLY_NEW_SHADER_SYNTAX 0

namespace pathos {

	static constexpr uint32 UBO_BINDING_INDEX = 1;
	static constexpr uint32 TEXTURE_UNIT = 0;

	struct UBO_Deferred_Pack_AlphaOnly {
		matrix4 modelView;
		matrix4 modelViewProj;
		matrix3x4 modelView3x3;
		vector4 color;
	};

#if ALPHAONLY_NEW_SHADER_SYNTAX
	class DeferredPackAlphaOnlyVS : public ShaderStage {
	public:
		DeferredPackAlphaOnlyVS() : ShaderStage(GL_VERTEX_SHADER, "DeferredPackAlphaOnlyVS") {
			addDefine("VERTEX_SHADER 1");
			setFilepath("deferred_pack_alphaonly.glsl");
		}
	};

	class DeferredPackAlphaOnlyFS : public ShaderStage {
	public:
		DeferredPackAlphaOnlyFS() : ShaderStage(GL_FRAGMENT_SHADER, "DeferredPackAlphaOnlyFS") {
			addDefine("FRAGMENT_SHADER 1");
			setFilepath("deferred_pack_alphaonly.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_Deferred_Pack_AlphaOnly, DeferredPackAlphaOnlyVS, DeferredPackAlphaOnlyFS);
#endif

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
		uboData.modelViewProj = inCamera->getViewProjectionMatrix() * modelMatrix;
		uboData.modelView3x3  = matrix3x4(uboData.modelView);
		uboData.color         = vector4(colorPtr[0], colorPtr[1], colorPtr[2], 1.0f);
		ubo.update(cmdList, UBO_BINDING_INDEX, &uboData);

		// Bind texture
		cmdList.bindTextureUnit(TEXTURE_UNIT, material->getTexture());

		// Drawcall
		inMesh->activate_position_uv_normal(cmdList);
		inMesh->activateIndexBuffer(cmdList);
		inMesh->drawPrimitive(cmdList);
	}

	void MeshDeferredRenderPass_Pack_AlphaOnly::createProgram() {
#if ALPHAONLY_NEW_SHADER_SYNTAX
		program = FIND_SHADER_PROGRAM(Program_Deferred_Pack_AlphaOnly);
#else
		Shader vs(GL_VERTEX_SHADER, "DeferredPackAlphaOnlyVS");
		vs.addDefine("VERTEX_SHADER 1");
		vs.loadSource("deferred_pack_alphaonly.glsl");

		Shader fs(GL_FRAGMENT_SHADER, "DeferredPackAlphaOnlyFS");
		fs.addDefine("FRAGMENT_SHADER 1");
		fs.loadSource("deferred_pack_alphaonly.glsl");

		program = pathos::createProgram(vs, fs, "Program_Deferred_Pack_AlphaOnly");
#endif

		ubo.init<UBO_Deferred_Pack_AlphaOnly>();
	}

}
