#include "material_shader.h"
#include "material_shader_assembler.h"
#include "shader_program.h"

namespace pathos {

	void MaterialShader::generateShaderProgram(const std::string& filepath, const MaterialTemplate* materialTemplate) {
		std::vector<std::string> sourceVS = materialTemplate->sourceLines;
		std::vector<std::string> sourceFS = materialTemplate->sourceLines;
		sourceVS[materialTemplate->lineIx_shaderstage] = "#define VERTEX_SHADER 1\n";
		sourceFS[materialTemplate->lineIx_shaderstage] = "#define FRAGMENT_SHADER 1\n";
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

		// #todo-material-assembler-fatal: 'No vertex shader bound' error
		// if a drawcall uses this program ???
		ShaderProgram* programPtr = program;
		ENQUEUE_RENDER_COMMAND([programPtr](RenderCommandList& cmdList) {
			programPtr->checkFirstLoad();
		});
		FLUSH_RENDER_COMMAND(true);

		uboName = "UBO_" + name;
		uboMaterial.init(uboTotalBytes, uboName.c_str());
	}

	void MaterialShader::fillUniformBuffer(uint8* uboMemory) {
		for (const MaterialConstantParameter& param : constantParameters) {
			switch (param.datatype) {
			case EMaterialParameterDataType::Float:
			{
				float* ptr = (float*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.fvalue[i];
			}
				break;
			case EMaterialParameterDataType::Int:
			{
				int32* ptr = (int32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.ivalue[i];
			}
				break;
			case EMaterialParameterDataType::Uint:
			{
				uint32* ptr = (uint32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.uvalue[i];
			}
				break;
			case EMaterialParameterDataType::Bool:
			{
				uint32* ptr = (uint32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = (uint32)param.bvalue[i];
			}
				break;
			default:
				CHECK_NO_ENTRY();
				break;
			}
		}
	}

	void MaterialShader::setParameterFloat(const char* name, float value) {
		for (MaterialConstantParameter& mcp : constantParameters) {
			if (mcp.name == name) {
				mcp.fvalue[0] = value;
				break;
			}
		}
	}

	void MaterialShader::setParameterVec3(const char* name, const vector3& value) {
		for (MaterialConstantParameter& mcp : constantParameters) {
			if (mcp.name == name) {
				mcp.fvalue[0] = value.x;
				mcp.fvalue[1] = value.y;
				mcp.fvalue[2] = value.z;
				break;
			}
		}
	}

}
