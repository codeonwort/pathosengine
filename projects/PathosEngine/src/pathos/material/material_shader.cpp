#include "material_shader.h"
#include "material_shader_assembler.h"
#include "pathos/material/material.h"
#include "pathos/rhi/shader_program.h"

namespace pathos {

	void MaterialShader::generateShaderProgram(const MaterialTemplate* materialTemplate, bool isHotReload) {
		std::vector<std::string> sourceVS = materialTemplate->sourceLines;
		std::vector<std::string> sourceFS = materialTemplate->sourceLines;
		sourceVS[materialTemplate->lineIx_shaderstage] = "#define VERTEX_SHADER 1\n";
		sourceFS[materialTemplate->lineIx_shaderstage] = "#define FRAGMENT_SHADER 1\n";
		nameVS = materialName + "VS";
		nameFS = materialName + "FS";
		programHash = COMPILE_TIME_CRC32_STR(materialName.c_str());

		ShaderStage* VS = new ShaderStage(GL_VERTEX_SHADER, nameVS.c_str());
		ShaderStage* FS = new ShaderStage(GL_FRAGMENT_SHADER, nameFS.c_str());
		VS->setSourceCode(nameVS + ".glsl", std::move(sourceVS));
		FS->setSourceCode(nameFS + ".glsl", std::move(sourceFS));

		ShaderProgram* oldProgram = ShaderDB::get().findProgram(programHash, false);
		if (oldProgram != nullptr) {
			ShaderDB::get().unregisterProgram(programHash);
			ENQUEUE_DEFERRED_RENDER_COMMAND([oldProgram](RenderCommandList& cmdList) {
				cmdList.registerDeferredCleanup(oldProgram);
			});
		}

		constexpr bool bIsMaterialProgram = true;
		program = new ShaderProgram(materialName.c_str(), programHash, bIsMaterialProgram);
		program->addShaderStage(VS);
		program->addShaderStage(FS);

		ShaderProgram* programPtr = program;
		ENQUEUE_RENDER_COMMAND([programPtr](RenderCommandList& cmdList) {
			programPtr->checkFirstLoad();
		});
		FLUSH_RENDER_COMMAND(true);

		if (uboTotalBytes > 0) {
			uboName = "UBO_" + materialName;
			uboMaterial.init(uboTotalBytes, uboName.c_str());
		}
	}

	void MaterialShader::extractMaterialParameters(
		std::vector<MaterialConstantParameter>& outConstants,
		std::vector<MaterialTextureParameter>& outTextures)
	{
		outConstants = constantParameters;
		outTextures = textureParameters;
	}

	uint32 MaterialShader::getNextInstanceID() {
		// #todo-material: Needs atomic?
		uint32 id = lastInstanceID;
		++lastInstanceID;
		return id;
	}

}
