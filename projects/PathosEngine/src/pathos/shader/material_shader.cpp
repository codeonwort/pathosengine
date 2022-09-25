#include "material_shader.h"
#include "material_shader_assembler.h"
#include "shader_program.h"

namespace pathos {

	void MaterialShader::generateShaderProgram(const std::string& filepath, const MaterialTemplate* materialTemplate) {
		std::vector<std::string> sourceVS = materialTemplate->sourceLines;
		std::vector<std::string> sourceFS = materialTemplate->sourceLines;
		sourceVS[materialTemplate->lineIx_shaderstage] = "#define VERTEX_SHADER 1";
		sourceFS[materialTemplate->lineIx_shaderstage] = "#define FRAGMENT_SHADER 1";
		nameVS = name + "VS";
		nameFS = name + "FS";
		programHash = COMPILE_TIME_CRC32_STR(name.c_str());

		ShaderStage* VS = new ShaderStage(GL_VERTEX_SHADER, nameVS.c_str());
		ShaderStage* FS = new ShaderStage(GL_FRAGMENT_SHADER, nameFS.c_str());
		VS->setSourceCode(nameVS + ".glsl", std::move(sourceVS));
		FS->setSourceCode(nameFS + ".glsl", std::move(sourceFS));

		constexpr bool bIsMaterialProgram = true;
		program = new ShaderProgram(name.c_str(), programHash, bIsMaterialProgram);
		program->addShaderStage(VS);
		program->addShaderStage(FS);
	}

}
